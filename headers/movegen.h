#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "../headers/board.h"
#include "../headers/array.h"
#include "../headers/move.h"
#include "../headers/array.h"

typedef struct magic_entry{
    U64 magic_num;
    U64 span;
    int n_bits_in_span;
} magic_entry;

extern U64 occupied;

void populate_attack_sets();

void generate_moves(dynamic_array* moves_array, int captures_only);

#endif
