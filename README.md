# Stream Syncer

Stream syncer is a small tool intended to merge and synchronize multiple stream files with different or identical sample rates to a csv file with timestamps and the highest sample rate of the original files. Usually it can sync several input streams and generate resulting csv files in just a few seconds (depending on system, highest sample rate, size of input streams).

Please note that not all types of input errors are checked and caught!

For synchronization simple upsampling is used (samples of streams with lower sample rates are just repeated until a new sample is available). The start timestamp used to generate timestamps as well as the stream locations and samplerates are defined in a json file.

## Used libraries

This project includes and uses following header only MIT licensed libraries:

* "JSON for Modern C++" https://github.com/nlohmann/json
* "Date" https://github.com/HowardHinnant/date

The MIT licenses are included in the directory of the library or in the file header.

No other (external) libraries are required by this software.

## Building information

This program is written in C++ 2020 and using cmake for building. OpenMP is activated for parallel execution of parts of the code and result in speed improvements.

The software was tested on Linux only using the gcc 12.2 compiler.

For building the executable in release mode you can use following commands for cmake:

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=RELEASE
cmake --build build
```

## Format of stream files

The stream files that should be synced should be encoded as text files. The values are expected to be of the datatype "float". At the end of one line (each line is a sample) there should be a "new line" char. If the streams are multidimensional it is expected that they are separated with a single white space.

## Configuration file

This is how a json configuration file may look like:

```json
{
	"starttime" : "2022-07-06 12:22:05.000 +0000",
	"streams" : [
		{"name": "ECG_1", "filename": "data/2022-07-06-12-25-08/03-h10-ecg.stream~", "sr" : 130.0, "dims" : 1, "dim_sel" : 0},
		{"name": "ECG_2", "filename": "data/2022-07-06-12-25-08/04-h10-ecg.stream~", "sr" : 130.0, "dims" : 1, "dim_sel" : 0},

		{"name": "ECG_HR_1", "filename": "data/2022-07-06-12-25-08/03-h10-hr.stream~", "sr" : 1.0, "dims" : 3, "dim_sel" : 0},
		{"name": "ECG_HR_2", "filename": "data/2022-07-06-12-25-08/04-h10-hr.stream~", "sr" : 1.0, "dims" : 3, "dim_sel" : 0},

		{"name": "ACC_1_x", "filename": "data/2022-07-06-12-25-08/03-h10-acc.stream~", "sr" : 50.0, "dims" : 3, "dim_sel" : 0},
		{"name": "ACC_1_y", "filename": "data/2022-07-06-12-25-08/03-h10-acc.stream~", "sr" : 50.0, "dims" : 3, "dim_sel" : 1},
		{"name": "ACC_1_z", "filename": "data/2022-07-06-12-25-08/03-h10-acc.stream~", "sr" : 50.0, "dims" : 3, "dim_sel" : 2},

		{"name": "ACC_2_x", "filename": "data/2022-07-06-12-25-08/04-h10-acc.stream~", "sr" : 50.0, "dims" : 3, "dim_sel" : 0},
		{"name": "ACC_2_y", "filename": "data/2022-07-06-12-25-08/04-h10-acc.stream~", "sr" : 50.0, "dims" : 3, "dim_sel" : 1},
		{"name": "ACC_2_z", "filename": "data/2022-07-06-12-25-08/04-h10-acc.stream~", "sr" : 50.0, "dims" : 3, "dim_sel" : 2},

		{"name": "GPS_LAT", "filename": "data/2022-07-06-12-25-08/03-phone-gps.stream~", "sr" : 1.0, "dims" : 2, "dim_sel" : 1},
		{"name": "GPS_LON", "filename": "data/2022-07-06-12-25-08/03-phone-gps.stream~", "sr" : 1.0, "dims" : 2, "dim_sel" : 0},

		{"name": "SB_HUMIDITY", "filename": "data/2022-07-06-12-25-08/03-sensorbox-humidity.stream~", "sr" : 1.0, "dims" : 1, "dim_sel" : 0},
		{"name": "SB_TEMPERATURE", "filename": "data/2022-07-06-12-25-08/03-sensorbox-temperature.stream~", "sr" : 1.0, "dims" : 1, "dim_sel" : 0}
	]
}
```

### Config file parameters

#### starttime

Timestamp used as initial time for the timestamp generation. As sample rate the highest sample rate (sr) of the provided streams is used.

#### streams

Array of streams that should be synchronized.

#### name

The name is used as the column name in the resulting csv file.

#### filename

The filename provides the location of the original stream file.

#### sr

The sample rate of the original file has to be provided with this variable.

#### dims

The dimension (e.g. "3" for 3-axis accelerometer data) of the original file has to be provided here.

#### dim_sel

Here the dimension that should be used in the synchronized csv file can be selected. Please note that you can just select one dimension here. So if you want to synchronize several dimensions you have to add it additionally with a different "name" and "dim_sel" attribute.

## Command line parameters

* argument 1: path to the configuration file
* argument 2: path to the output csv file (directories are not created)

## Batch processing example

All json configuration files in the "synced_data" directory are passed to "stream_syncer" executable and the resulting csv files are placed in the same directory as the config files. Afterwards all csv files are compressed using parallelized gzip compression.

```bash
for i in synced_data/*.json; do ./stream_syncer "$i" "${i%.*}.csv"; done

for i in synced_data/*.csv; do pigz -9 "$i"; done
```

## License

This software is licensed under the MIT License

Copyright (c) 2022-2023 Dr. Andreas Seiderer

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
