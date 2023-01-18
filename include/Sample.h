#pragma once

/**
 * (c)2022 Andreas Seiderer
 */

#include <string>
#include <variant>

class Sample {
    public:
        Sample() : timestamp_ns(0L) {}

        Sample(uint64_t t, std::string v) {
            timestamp_ns = t;
            value = v;
        }

        Sample(uint64_t t, float v) {
            timestamp_ns = t;
            value = v;
        }

        std::string getStringVal() {
            return std::get<std::string>(value);
        }

        float getFloatVal() {
            return std::get<float>(value);
        }

        uint64_t getTimestamp() {
            return timestamp_ns;
        }

        void setVal(std::string v) {
            value = v;
        }

        void setVal(float v) {
            value = v;
        }

        void setTimestamp(uint64_t t) {
            timestamp_ns = t;
        }

    private:
        std::variant<float, std::string> value;
        uint64_t timestamp_ns;
};