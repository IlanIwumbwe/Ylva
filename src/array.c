#include "../headers/array.h"

/// @brief Allocate memory for the dynamic array
/// @param da Pointer to dynamic array
/// @param capacity Number of int slots that should be allocated
void init_da(dynamic_array* da, size_t capacity){
    if(capacity == 0){
        printf("Cannot initialise array with capacity of 0 bytes!");
        exit(-1);
    } else {
        da->array = malloc(sizeof(int) * capacity); 

        if(da->array != NULL){
            da->used = 0;
            da->capacity = capacity;
        } else {
            printf("Allocation of memory for dynamic array failed!");
            exit(-1);
        }

    } 
}

void da_append(dynamic_array* da, int element){
    if(da->used == da->capacity){
        da->capacity = 2 * da->capacity;
        int* new_block = realloc(da->array, sizeof(int) * da->capacity);

        if(new_block == NULL){
            printf("Reallocation of memory for dynamic array failed!");
            exit(-1);
            free_da(da);
        } else {
            da->array = new_block;
        }
    }

    da->array[da->used++] = element;
}

void free_da(dynamic_array* da){
    free(da->array);
    da->capacity = da->used = 0;
    da->array = NULL;
}

void reset_da(dynamic_array* da){
    da->used = 0;
}