#ifndef DEFS_H
#define DEFS_H

#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define MOVE_FORMAT std::regex(R"([a-h][1-8][a-h][1-8](q|r|n|b)?)")

#define CAPTURE_FLAG 0x4000
#define PROMO_FLAG 0x8000

#define RANK(rank_num) (uint64_t)(0xff << (rank_num-1))
#define A_FILE 0x8080808080808080
#define H_FILE 0x0101010101010101

#include "helper_funcs.h"

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

typedef enum{
    PVE,    
    EVE,
    PVP
} game_modes;

std::vector<std::pair<char, piece_names>> namecharint = {
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

std::unordered_map<std::string, unsigned int> promo_flags = {
    {"q", 11},
    {"r", 10},
    {"n", 8},
    {"b", 9},
    {"qc", 15},
    {"rc", 14},
    {"nc", 12},
    {"bc", 13}
};

#endif
