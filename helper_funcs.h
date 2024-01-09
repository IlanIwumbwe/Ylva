#ifndef HELPER_FUNCS_H
#define HELPER_FUNCS_H

#include <regex>

// common imports
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <bitset>
#include <cstdint>
#include <cmath>
#include <unordered_map>

namespace fs = std::filesystem;

std::vector<std::string> splitString(const std::string& input, const char& delimiter) {
    std::istringstream iss(input);
    std::vector<std::string> parts;
    std::string part;

    while (std::getline(iss, part, delimiter)) {
        parts.push_back(part);
    }

    return parts;
}

inline std::string removeWhiteSpace(std::string str){
    std::regex pattern(R"(^\s+|\s+$|\t|\n)");

    return std::regex_replace(str, pattern, "");
}

inline auto numtobin(const uint64_t& number){
    std::bitset<64> binaryRepresentation(number);

    return binaryRepresentation;
}

inline void printbitboard(const uint64_t& number){
    auto bin = numtobin(number);
    std::cout << "  ------------------------" << std::endl;
    for(int i = 0; i < 64; ++i){
        if((i+1) % 8 == 0){
            std::cout << " " << bin.to_string()[i] << " \n";
        }else if(i % 8 == 0){
            std::cout << "   " << bin.to_string()[i] << " ";
        }else{
            std::cout << " " << bin.to_string()[i] << " ";
        }
    }
    std::cout << "  ------------------------" << std::endl;
}

int alg_to_int(const std::string& square){
    return 8*((square[1]-'0')-1) + (104-square[0]);
}

std::string to_lower(const std::string input){
    std::string input_lower = input;

    for (auto& x : input_lower){
        x = (char)tolower(x);
    }
    return input_lower;
}

inline int hexstringtoint(const std::string& hexString) {
    std::istringstream iss(hexString);
    int result;
    iss >> std::hex >> result;
    return result;
}

inline bool isvalidHex(const std::string& hexString){
    auto pattern = std::regex(R"(0x([0-9a-fA-F])+)");

    return std::regex_match(hexString, pattern);
}

inline bool isvalidNum(const std::string& numString){
    auto pattern = std::regex(R"(0x([0-9a-fA-F])+|\-?([0-9])+)");

    return std::regex_match(numString, pattern);
}

/// Convert a base 10 or hex string into integer
inline int stringtoint(const std::string& string){
    if(isvalidHex(string)){
        return hexstringtoint(string);
    } else {
        return std::stoi(string);
    }
}

#endif
