#ifndef ZOBRIST_H
#define ZOBRIST_H

#include "utils.h"
#include "board.h"

typedef struct szobrist{
    U64 piece_zobrist_keys[13][64];
    U64 turn_key;
    U64 castling_key[16];
} zobrist_info;

extern zobrist_info zi;

void init_hash_keys(void);

void init_hash(board_state* state);

void modify_hash_by_castling_rights(board_state* state, U16 old_castling_rights);

#endif
