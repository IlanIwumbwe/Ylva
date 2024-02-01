#include "defs.h"

namespace fs = std::filesystem;

dirInfo dir_info[8] = {
    {north, 8, RANK(8), south},
    {east, -1, H_FILE, west},
    {west, 1, A_FILE, east},
    {south, -8, RANK(1), north},
    {noEa, 7, H_FILE | RANK(8), soWe},
    {noWe, 9, A_FILE  | RANK(8), soEa},
    {soEa, -9, H_FILE | RANK(1), noWe},
    {soWe, -7, A_FILE | RANK(1), noEa}    
};

std::unordered_map<char, piece_names> char_to_name = {
    {'P', P},
    {'K', K},
    {'Q', Q},
    {'R', R},
    {'N', N},
    {'B', B},
    {'p', p},
    {'k', k},
    {'q', q},
    {'r', r},
    {'n', n},
    {'b', b}
};

std::unordered_map<std::string, uint> promo_flags = {
    {"q", 11},
    {"r", 10},
    {"n", 8},
    {"b", 9},
    {"qc", 15},
    {"rc", 14},
    {"nc", 12},
    {"bc", 13}
};

uint p_flags[4] = {8,9,10,11};
uint pc_flags[4] = {12,13,14,15};
U64 knight_attack_set[64] = {};
U64 king_attack_set[64] = {};
U64 RAYS[8][64] = {};

void populate_rays(){
    for(auto info : dir_info){
        for(int i = 0; i < 64; ++i){
            U64 out = 0;
            int mult = 0;

            out |= set_bit(i);

            while((out & info.bound) == 0){
                mult ++;
                out |= set_bit(i + mult * info.offset);
            }

            out &= ~set_bit(i);

            RAYS[info.dir][i] = out;
        }
    }
}

void populate_attack_sets(){
    for(int i = 0; i < 64; ++i){
        knight_attacks(set_bit(i), knight_attack_set[i]);
        king_attacks(set_bit(i), king_attack_set[i]);
    }
}

char name_to_char(const piece_names& name){
    switch(name){
        case P : return 'P';
        case p : return 'p';
        case Q : return 'Q';
        case q : return 'q';
        case N : return 'N';
        case n : return 'n';
        case R : return 'R';
        case r : return 'r';
        case B : return 'B';
        case b : return 'b';
        case K : return 'K';
        case k : return 'k';
        case None : return '.';
        default :
            std::cout << "What? The piece name (piece_name) " << name << " does not exist" << std::endl;
            exit(0);
    }
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

std::vector<std::string> splitString(const std::string& input, const char& delimiter) {
    std::istringstream iss(input);
    std::vector<std::string> parts;
    std::string part;

    while (std::getline(iss, part, delimiter)) {
        parts.push_back(part);
    }

    return parts;
}

std::string removeWhiteSpace(std::string str){
    std::regex pattern(R"(^\s+|\s+$|\t|\n)");

    return std::regex_replace(str, pattern, "");
}

bool isStringDigit(std::string& input){
    for(auto c : input){
        if(!isdigit(c)){
            return false;
        }
    }

    return true;

}

inline auto numtobin(const U64& number){
    std::bitset<64> binaryRepresentation(number);

    return binaryRepresentation;
}

void printbitboard(const U64& number){
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

std::string int_to_alg(const uint& square){
    char fileind = 104 - (square % 8);
    int rankind = (square / 8) + 1;

    std::string out(1,fileind);
    out += std::to_string(rankind);
    return out;
}

uint count_set_bits(U64 bitboard){
    uint count = 0;
    
    for(; bitboard; ++count){
        bitboard &= bitboard-1;
    }

    return count;
}