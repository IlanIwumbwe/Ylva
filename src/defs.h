#ifndef DEFS_H
#define DEFS_H

#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define MOVE_FORMAT std::regex(R"([a-h][1-8][a-h][1-8](q|r|n|b)?)")

#define CAPTURE_FLAG 0x4000
#define PROMO_FLAG 0x8000

#define RANK(rank_num) ((uint64_t)0xff << (rank_num-1)*8)
#define A_FILE 0x8080808080808080
#define B_FILE 0x4040404040404040
#define G_FILE 0x0202020202020202
#define H_FILE 0x0101010101010101

#define K_castle 0xf000
#define Q_castle 0x0f00
#define k_castle 0x00f0
#define q_castle 0x000f

#define get_lsb(bitboard) __builtin_ctzll(bitboard)
#define set_bit(i) (1ULL << (i))

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

unsigned int p_flags[4] = {8,9,10,11};
unsigned int pc_flags[4] = {12,13,14,15};

// attack sets
uint64_t knight_attack_set[64];
uint64_t king_attack_set[64];

void populate_attack_sets(){
    for(int i = 0; i < 64; ++i){
        knight_attacks(set_bit(i), knight_attack_set[i]);
        king_attacks(set_bit(i), king_attack_set[i]);
    }
}

#endif
