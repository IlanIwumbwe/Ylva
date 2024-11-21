#ifndef POSITIONS_H
#define POSITIONS_H

#include "utils.h"
#define N_POSITIONS 2

typedef struct spos {
    char* fen;
    int results[MAX_SEARCH_DEPTH];
} test_pos;

extern test_pos ts[N_POSITIONS];

#endif