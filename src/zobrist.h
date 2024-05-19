#ifndef ZORBIST_H
#define ZOBRIST_H

#include "defs.h"

extern U64 Piece_keys[13][64];
extern U64 Turn_key;
extern U64 Castle_key[16];

void init_hash_keys();

#endif
