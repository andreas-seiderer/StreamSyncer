
/**
 * (c)2022 Andreas Seiderer
 */


#include <iostream>
#include <fstream>
#include <chrono>
#include <memory>
#include <map>

#include "include/date.h"
#include "include/Stream.h"
#include "include/Sample.h"

#include "include/nlohmann/json.hpp"

using json = nlohmann::json;


/**
 * @brief Split a string with a specific delimiter
 * 
 * @param s Input string
 * @param del Delimiter String
 * @param strvec Output String vector
 */
void stringsplit(std::string s, std::string del, std::vector<std::string>* strvec)
{
    int start, end = -1*del.size();
    do {
        start = end + del.size();
        end = s.find(del, start);

        strvec->push_back(s.substr(start, end - start));
    } while (end != -1);
}


/**
 * @brief Reading in ASCII encoded data stream where multiple dimensions are split with a whitespace. Timestamps with millisecond resolution are generated with the samplerate and the start time.
 * 
 * @param s Variable of type "Stream" to read the data into.
 * @param filepath Filepath of the data stream to read in.
 * @param start_time Start time used for the generated timestamps.
 * @param name Name of the stream.
 * @param sr Sample rate of the stream. Used to generate the timestamps.
 * @param dims Total dimensions of the read in data stream.
 * @param dimsel Dimension to be used for the output stream. It will always be one dimensional.
 * @return true If the file to be read in exists.
 * @return false If the file to be read in doesn't exist.
 */
bool read_float_stream(Stream *s, std::string filepath, uint64_t start_time, std::string name, float sr, uint8_t dims, uint8_t dimsel) {

    if (!std::filesystem::exists(filepath)) {
        return false;
    }

    std::ifstream FileReader(filepath);
    std::string line;

    s->setName(name);
    s->setSR(sr);

    float timestep_ms = (1.f/sr) * 1000.f;
    uint64_t i = 0;

    while (std::getline (FileReader, line, '\n')) {
        
        float val = 0.f;

        if (dims == 1) {
            val = std::stof(line);
        }
        else if (dims > 1) {
            std::vector<std::string> strs;

            //remove new line and space
            line.erase(line.length()-2);

            stringsplit(line, " ", &strs);

            val = std::stof(strs.at(dimsel));
        }

        s->appendSample(Sample(start_time + (uint64_t)(timestep_ms*(float)i), val));

        i++;
    }

    FileReader.close();

    return true;
}


/**
 * @brief Count the number of zeros in the stream.
 * 
 * @param s Input stream.
 * @return uint64_t Count of zeros.
 */
uint64_t total_zero_count(Stream *s) {
    uint64_t c = 0;

    #pragma omp for
    for (Sample v : *s->getSamples()) {
            if(v.getFloatVal() == 0.f)
                c+=1;
    }

    return c;
}


/**
 * @brief Print some informations about the stream.
 * 
 * @param s Input stream.
 */
void print_stream_info(Stream *s) {
    std::cout << s->getName() << ": samples: " << s->getSamples()->size() <<
     " (" << (int)(s->getSamples()->size()/s->getSR())  << " sec) zero values: " << total_zero_count(s) << std::endl;
}


/**
 * @brief Creates a data table and writes it to a csv file where all input streams are synchronized. Synchronization is timestamp based by upsampling to the highest samplerate of the streams.
 * 
 * @param streams Array of streams to be synchronized.
 * @param size Size of the array of streams.
 * @param outfile Path to the output csv file.
 */
void create_data_table(Stream* streams[], uint32_t size, std::string outfile) {

    // calculate maximum row count and set colnames
    uint64_t cols = size;
    uint64_t rows = 0;
    uint64_t max_idx = -1;
    std::string* colnames = new std::string[cols];

    for (uint64_t i = 0; i < size; i++) {
        auto s = streams[i]->getSamples()->size();
        colnames[i] = streams[i]->getName();
        if (s > rows) {
            rows = s;
            max_idx = i;
        }
    }   
    
    uint64_t* timestamps = new uint64_t[rows];

    // copy timestamps from stream with most values
    #pragma omp for
    for (uint64_t i = 0; i < rows; i++) {
        timestamps[i] = streams[max_idx]->getSamples()->at(i).getTimestamp();
    }

    // initialize data table
    float** arr = new float*[rows];

    #pragma omp for
    for (uint64_t i = 0; i < rows; i++) {
        arr[i] = new float[cols];
    }


    // synchronize data
    for (uint64_t j = 0; j < cols; j++) {
        
        uint64_t stream_row = 0;
        uint64_t t = 0;
        uint64_t t_c = 0;

        #pragma omp for
        for (uint64_t i = 0; i < rows; i++) {
            t = timestamps[i];
            t_c = streams[j]->getSamples()->at(stream_row).getTimestamp();

            while(stream_row < streams[j]->getSamples()->size()-1 && t  > t_c) {
                stream_row++;

                t = timestamps[i];
                t_c = streams[j]->getSamples()->at(stream_row).getTimestamp();
            }

            if (t <= t_c) {
                arr[i][j] = streams[j]->getSamples()->at(stream_row).getFloatVal();
            }
        }
    }


    // write to csv
    std::ofstream out(outfile);

    out << "timestamp;";

    for (uint64_t i = 0; i < cols; i++) {
        out << colnames[i] << ";";
    }
    out << '\n';

    for (uint64_t i = 0; i < rows; i++)
    {
        out << timestamps[i] << ";";
        for (uint64_t j = 0; j < cols; j++) {
            out << arr[i][j] << ";";
        }
        out << '\n';
    }

    out.close();

    // clean up
    #pragma omp for
    for (uint64_t i = 0; i < rows; i++) {
        delete[] arr[i];
    }
    delete[] arr;


    delete[] colnames;
    delete[] timestamps;

}


/*
argument 1 = config file; argument 2 = csv output file

Format of config file:

{
	"starttime" : "2022-07-06 12:22:05.000 +0000",
	"streams" : [
		{"name": "ECG_1", "filename": "../data/2022-07-06-12-25-08/03-h10-ecg.stream~", "sr" : 130.0, "dims" : 1, "dim_sel" : 0}
	]
}

*/
int main(int argc, char** args) {

    if (argc != 3) {
        std::cout << "Wrong number of arguments." << std::endl;
        return -1;
    }

    std::cout << "Using openMP " << _OPENMP << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    std::string config_file = args[1];
    std::string output_file = args[2];

    std::ifstream f(config_file);
    json data = json::parse(f);

    std::cout << "start time from config: " << data["starttime"] << std::endl;

    {
        std::vector<std::string> stream_names;
        std::map<std::string, std::unique_ptr<Stream>> m_streams;

        // parse start date / time
        std::istringstream in(data["starttime"].get<std::string>());
        std::chrono::sys_time<std::chrono::milliseconds> tp_start;
        in >> date::parse("%Y-%m-%d %T %z", tp_start);


        // create and read in streams
        for (auto& [key, value] : data["streams"].items()) {
            std::cout << key << " : " << value << "\n";

            stream_names.push_back(value["name"]);
            m_streams[value["name"]] = std::unique_ptr<Stream> (new Stream());

            bool result = read_float_stream(m_streams[value["name"]].get(), 
                value["filename"], tp_start.time_since_epoch().count(), value["name"], value["sr"], value["dims"], value["dim_sel"]);

            if (!result) {
                std::cout << "\033[1;31m Error: File " << value["filename"] << " not found!\033[0m\n";
                return -1;
            }
        }
         

        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = duration_cast<std::chrono::microseconds>(stop - start);

        std::cout << "reading files took: " << duration.count() << " µS\n" << std::endl;

        // print stream info
        for (auto const& element : m_streams) {
            print_stream_info(element.second.get());
        }


        start = std::chrono::high_resolution_clock::now();

        auto streams = new Stream*[stream_names.size()];
        
        uint32_t i = 0;
        for (auto n : stream_names) {
            streams[i] = m_streams[n].get();
            i++;
        }

        create_data_table(streams, stream_names.size(), output_file);

        delete[] streams;

        stop = std::chrono::high_resolution_clock::now();
        duration = duration_cast<std::chrono::microseconds>(stop - start);

        std::cout << "\nsynchronizing / writing to csv took: " << duration.count() << " µS" << std::endl;

    }

    return 0;
}
