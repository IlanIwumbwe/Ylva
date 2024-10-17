#include "../headers/array.h"

/// @brief Allocate memory for the dynamic array
/// @param da Pointer to dynamic array
/// @param capacity Number of int slots that should be allocated
void init_da(dynamic_array* da, size_t capacity){
    if(capacity == 0){
        fprintf(stderr, "Cannot initialise array with capacity of 0 bytes!");
        exit(-1);
    } else {
        da->array = malloc(sizeof(Move) * capacity); 

        if(da->array != NULL){
            da->used = 0;
            da->capacity = capacity;
        } else {
            fprintf(stderr, "Allocation of memory for dynamic array failed!");
            exit(1);
        }

    } 
}

/// @brief Append a move to the moves array
/// @param da 
/// @param move 
void da_append(dynamic_array* da, Move move){
    if(da->used == da->capacity){
        da->capacity = 2 * da->capacity;
        Move* new_block = realloc(da->array, sizeof(Move) * da->capacity);

        if(new_block == NULL){
            fprintf(stderr, "Reallocation of memory for dynamic array failed!");
            exit(1);
            free_da(da);
        } else {
            da->array = new_block;
        }
    }

    da->array[da->used++] = move;
}

/// @brief Free memory used by the moves array
/// @param da 
void free_da(dynamic_array* da){
    free(da->array);
    da->capacity = da->used = 0;
    da->array = NULL;
}

/// @brief Initialise pv table 
/// @param pvt pointer to pv table
/// @param capacity number of entries to put into pv table
void init_pv(pv_table* pvt, size_t capacity){
    if(capacity == 0){
        fprintf(stderr, "Cannot initialise array with capacity of 0 bytes!");
        exit(-1);
    }

    pvt->table = (pv_entry*) malloc(sizeof(pv_entry) * capacity);

    if(pvt->table != NULL){
        reset_pv_entries(pvt);
        pvt->capacity = capacity;
    } else {
        fprintf(stderr, "Allocation of memory for dynamic array failed!");
        exit(1);
    }
}

/// @brief Set all pv entries in the table to 0
/// @param pvt 
void reset_pv_entries(pv_table* pvt){
    for(size_t i = 0; i < pvt->capacity; ++i){
        pvt->table[i].key = 0ULL;
        pvt->table[i].move = 0;
    }
}

/// @brief create an entry using the key and move given, and store it into the pv table
/// @param pvt 
/// @param key 
/// @param move 
void store_pv_entry(pv_table* pvt, U64 key, U16 move){

    int index = key % pvt->capacity;

    pv_entry e = {.key = key, .move = move};

    pvt->table[index] = e;
}

/// @brief index into pv table using key, if a move has been stored for this position in the pv table, return it, else return 0
/// @param pvt 
/// @param key 
/// @return 
U16 probe_pv_table(pv_table* pvt, U64 key){
    int index = key % pvt->capacity;

    if(pvt->table[index].key == key){
        return pvt->table[index].move;
    }

    return 0;
}

void free_pv(pv_table* pvt){
    free(pvt);
}

void clear_pv_array(U16* array){
    for(int i = 0; i < MAX_SEARCH_DEPTH; ++i){
        array[i] = 0;
    }
}