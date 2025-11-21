#pragma once
#include <vector>
#include <string>
#include <sstream>

class Parser {
public:
    static std::vector<double> parseVector(const std::string& line) {
        std::vector<double> vector;
        size_t pos = line.find(":");
        if (pos != std::string::npos) {
            std::string values = line.substr(pos + 1);
            values.erase(0, values.find_first_not_of(" \t"));
            std::istringstream iss(values);
            double value;
            while (iss >> value) {
                vector.push_back(value);
            }
        }
        return vector;
    }

    static std::string getValue(const std::string& line) {
        size_t pos = line.find(":");
        if (pos != std::string::npos) {
            std::string value = line.substr(pos + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            return value;
        }
        return "";
    }
}; 