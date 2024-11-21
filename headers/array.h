#ifndef ARRAY_H
#define ARRAY_H

#include "../headers/utils.h"
typedef struct sma {
    Move array[MAX_MOVES];
    size_t used;
} moves_array;

void ma_append(moves_array* ma, Move m);

void ma_reset(moves_array* ma);

#endif