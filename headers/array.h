#ifndef ARRAY_H
#define ARRAY_H

#include "../headers/utils.h"

/// @brief Principal variation entry in PV table stores best move assosiated with each position
typedef struct spv{
    U64 key;
    U16 move;
} pv_entry;

typedef struct spvt{
    pv_entry* table;
    size_t capacity;
} pv_table;

typedef struct sma {
    Move array[MAX_MOVES];
    size_t used;
} moves_array;

void ma_append(moves_array* ma, Move m);

void ma_reset(moves_array* ma);

void init_pv(pv_table* pvt, size_t capacity);

void reset_pv_entries(pv_table* pvt);

void store_pv_entry(pv_table* pvt, U64 key, U16 move);

U16 probe_pv_table(pv_table* pvt, U64 key);

void free_pv(pv_table* pvt);

void clear_pv_array(U16* array);

#endif