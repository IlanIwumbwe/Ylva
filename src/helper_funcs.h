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

inline auto numtobin(const U64& number){
    std::bitset<64> binaryRepresentation(number);

    return binaryRepresentation;
}

inline void printbitboard(const U64& number){
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

unsigned int count_set_bits(U64 bitboard){
    unsigned int count = 0;
    
    for(; bitboard; ++count){
        bitboard &= bitboard-1;
    }

    return count;
}

/// Produce bitboard of all attacked squares by a knight at given square
void knight_attacks(U64 bitboard, U64& output){
    output = 0;

    output |= (bitboard & ~(G_FILE | H_FILE | RANK(8))) << 6;
    output |= (bitboard & ~(RANK(7) | RANK(8) | H_FILE)) << 15;
    output |= (bitboard & ~(A_FILE | B_FILE | RANK(8))) << 10;
    output |= (bitboard & ~(A_FILE | RANK(7) | RANK(8))) << 17;
    output |= (bitboard & ~(G_FILE | H_FILE | RANK(1))) >> 10;
    output |= (bitboard & ~(H_FILE | RANK(1) | RANK(2))) >> 17;
    output |= (bitboard & ~(A_FILE | B_FILE | RANK(1))) >> 6;
    output |= (bitboard & ~(A_FILE | RANK(1) | RANK(2))) >> 15;
}

/// Produce bitboard of all attacked squares by a king at given square
void king_attacks(U64 bitboard, U64& output){
    output = 0;
    
    output |= (bitboard & ~RANK(8)) << 8;
    output |= (bitboard & ~H_FILE) >> 1;
    output |= (bitboard & ~A_FILE) << 1;
    output |= (bitboard & ~RANK(1)) >> 8;
    output |= (bitboard & ~(RANK(8) | H_FILE)) << 7;
    output |= (bitboard & ~(RANK(8) | A_FILE)) << 9;
    output |= (bitboard & ~(RANK(1) | H_FILE)) >> 9;
    output |= (bitboard & ~(RANK(1) | A_FILE)) >> 7;
}

#endif
