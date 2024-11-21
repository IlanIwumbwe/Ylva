#include "../headers/zobrist.h"

zobrist_info zi;

/// @brief Fill up zobrist keys as random 64 bit values
/// @param  
void init_hash_keys(void){
    int i, j;

    for(i = 0; i < 13; ++i){
        for(j = 0; j < 64; ++j){
            zi.piece_zobrist_keys[i][j] = RAND64;
        }
    }

    zi.turn_key = RAND64;

    for(i = 0; i < 16; ++i){
        zi.castling_key[i] = RAND64;
    }

}

/// @brief Initialise zobrist key for starting state
/// @param state 
void init_hash(board_state* state){
    square sq;
    U64 occupied = state->data->occupied;

    state->data->hash = 0ULL;

    while(occupied){
        sq = get_lsb(occupied);
        occupied &= (occupied - 1);

        state->data->hash ^= zi.piece_zobrist_keys[state->board[sq]][sq];
    }

    if(state->data->ep_square != s_none){
        state->data->hash ^= zi.piece_zobrist_keys[p_none][state->data->ep_square];
    }

    if(state->data->s == BLACK){
        state->data->hash ^= zi.turn_key;
    }

    state->data->hash ^= zi.castling_key[state->data->castling_rights & 0xf];
}

void modify_hash_by_castling_rights(board_state* state, U16 old_castling_rights){
    state->data->hash ^= old_castling_rights;
    state->data->hash ^= state->data->castling_rights;    
}   