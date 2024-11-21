#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "../headers/board.h"
#include "../headers/array.h"
#include "../headers/move.h"

typedef struct magic_entry{
    U64 magic_num;
    U64 span;
    int n_bits_in_span;
} magic_entry;

extern U64 occupied, whites, blacks;
extern int n_checkers;

void populate_attack_sets();

void generate_moves(board_state* state, moves_array* legal_moves, int captures_only);

#endif
