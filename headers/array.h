#ifndef ARRAY_H
#define ARRAY_H

#include "../headers/utils.h"

/// @brief Dynamic array that holds moves
typedef struct sda{
    Move* array;
    size_t capacity;
    size_t used; 
} dynamic_array;

void init_da(dynamic_array* da, size_t capacity);

void da_append(dynamic_array* da, Move move);

void free_da(dynamic_array* da);

#endif