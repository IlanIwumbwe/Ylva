#include "zobrist.h"

U64 Piece_keys[13][64];
U64 Turn_key;
U64 Castle_key[16];

void init_hash_keys(){
    for(int i = 0; i < 13; ++i){
        for(int j = 0; j < 64; ++j){
            Piece_keys[i][j] = RAND64;
        }
    }

    Turn_key = RAND64;

    for(int i = 0; i < 16; ++i){
        Castle_key[i] = RAND64;
    }
}

