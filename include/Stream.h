#pragma once

/**
 * (c)2022 Andreas Seiderer
 */

#include <vector>
#include "Sample.h"


class Stream {
    public:
        Stream() {}

        void setName(std::string n) {
            this->name = n;
        }

        std::string getName() {
            return name;
        }

        void setSR(float s) {
            this->sr = s;
        }

        float getSR() {
            return this->sr;
        }

        void appendSample(Sample s) {
            data.push_back(s);
        }

        std::vector<Sample>* getSamples() {
            return &data;
        }

    private:
        std::vector<Sample> data;
        std::string name;
        float sr;

};