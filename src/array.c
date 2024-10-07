#include "../headers/array.h"

/// @brief Allocate memory for the dynamic array
/// @param da 
/// @param capacity 
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

int da_append(dynamic_array* da, int element){
    if(da->used == da->capacity){
        da->capacity = 2 * da->capacity;
        da->array = realloc(da->array, sizeof(int) * da->capacity);

        if(da->array == NULL){
            printf("Reallocation of memory for dynamic array failed!");
            return -1;
        }
    }

    da->array[da->used++] = element;

    return 0;
}

void free_da(dynamic_array* da){
    da->array = NULL;
    da->capacity = da->used = 0;
    free(da->array);
}

void reset_da(dynamic_array* da){
    da->used = 0;
}