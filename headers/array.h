#ifndef ARRAY_H
#define ARRAY_H

#include "../headers/utils.h"

/// @brief Dynamic array that holds 16 bit values
typedef struct dynamic_array{
    int* array;
    size_t capacity;
    size_t used; 
} dynamic_array;

void init_da(dynamic_array* da, size_t capacity);

void da_append(dynamic_array* da, int element);

void free_da(dynamic_array* da);

void reset_da(dynamic_array* da);

#endif