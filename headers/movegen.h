#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "../headers/board.h"
#include "../headers/array.h"
#include "../headers/move.h"

#define A_FILE 0x8080808080808080
#define B_FILE 0x4040404040404040
#define G_FILE 0x0202020202020202
#define H_FILE 0x0101010101010101   
#define RANK(rank_num) ((uint64_t)0xff << (rank_num-1)*8)

typedef struct magic_entry{
    U64 magic_num;
    U64 span;
    int n_bits_in_span;
} magic_entry;

extern U64 occupied, whites, blacks;

void populate_attack_sets();

void generate_moves(board_state* state, moves_array* legal_moves, int captures_only);

#endif
