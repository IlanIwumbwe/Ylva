#ifndef DEFS_H
#define DEFS_H

#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define MOVE_FORMAT std::regex(R"([a-h][1-8][a-h][1-8](q|r|n|b)?)")

typedef uint64_t U64;

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
    PVP,
    PERFT
} game_modes;

typedef enum{
    north,
    east,
    west,
    south,
    noEa,
    soEa,
    noWe,
    soWe
} dirs;

typedef enum {
    diag,
    nondiag
} ray_type;

struct dirInfo {
    dirs dir;
    int offset;
    U64 bound;
    dirs opp_dir;
}; 

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
U64 knight_attack_set[64];
U64 king_attack_set[64];

void populate_attack_sets(){
    for(int i = 0; i < 64; ++i){
        knight_attacks(set_bit(i), knight_attack_set[i]);
        king_attacks(set_bit(i), king_attack_set[i]);
    }
}

U64 RAYS[8][64];

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

#endif
