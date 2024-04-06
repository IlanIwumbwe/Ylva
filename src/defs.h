#ifndef DEFS_H
#define DEFS_H

#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define MOVE_FORMAT std::regex(R"([a-h][1-8][a-h][1-8](q|r|n|b)?)")
#define VERSION_FORMAT std::regex(R"(v[0-2])")

#include <cstdint>
#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <regex>
#include <cmath>

typedef uint64_t U64;
typedef unsigned int uint;

#define CAPTURE_FLAG 0x4000
#define PROMO_FLAG 0x8000

#define A_FILE 0x8080808080808080
#define B_FILE 0x4040404040404040
#define G_FILE 0x0202020202020202
#define H_FILE 0x0101010101010101   
#define RANK(rank_num) ((uint64_t)0xff << (rank_num-1)*8)

#define K_castle 0x8
#define Q_castle 0x4
#define k_castle 0x2
#define q_castle 0x1        

#define get_lsb(bitboard) __builtin_ctzll(bitboard)
#define get_msb(bitboard) 64 - (__builtin_clzll(bitboard)+1)
#define set_bit(i) (1ULL << (i))
#define get_bit(bitboard, i) (bitboard & set_bit(i))

#define infinity std::numeric_limits<int>::max()

typedef enum {
    None = 0,
    P = 1 , 
    K = 2 ,
    Q = 3 , 
    R = 4 ,
    N = 5 ,
    B = 6 ,
    p = 9 ,
    k = 10,
    q = 11,
    r = 12,
    n = 13,
    b = 14,
} piece_names;

typedef enum{
    WHITE = 0,
    BLACK = -1,
} colour;

typedef enum{PVE, EVE, PVP, PERFT, BENCHMARK} game_modes;
 
typedef enum{north, east, west, south, noEa, soEa, noWe, soWe} dirs;

typedef enum {diag, nondiag} ray_type;

struct dirInfo {
    dirs dir;
    int offset;
    U64 bound;
    dirs opp_dir;
}; 

extern dirInfo dir_info[8];

extern std::unordered_map<char, piece_names> char_to_name;

extern std::unordered_map<std::string, uint> promo_flags;

extern std::unordered_map<piece_names, int> get_piece_value;

extern uint p_flags[4];
extern uint pc_flags[4];

// attack sets
extern U64 knight_attack_set[64];
extern U64 king_attack_set[64];

extern U64 RAYS[8][64];

void populate_attack_sets();

void populate_rays();

char name_to_char(const piece_names& name);

void knight_attacks(U64 bitboard, U64& output);

void king_attacks(U64 bitboard, U64& output);

std::vector<std::string> splitString(const std::string& input, const char& delimiter);

std::string removeWhiteSpace(std::string str);

bool isStringDigit(std::string& input);

auto numtobin(const U64& number);

void printbitboard(const U64& number);

int alg_to_int(const std::string& square);

std::string int_to_alg(const uint& square);

uint count_set_bits(U64 bitboard);

#endif
