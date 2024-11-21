#include "../headers/array.h"

void ma_append(moves_array* ma, Move m){
    if(ma->used < MAX_MOVES){
        ma->array[ma->used++] = m;
    } else {
        fprintf(stderr, "Moves array already full!");
        exit(-1);
    }
}

void ma_reset(moves_array* ma){
    ma->used = 0;
}

