#ifndef ZORBIST_H
#define ZOBRIST_H

#include "defs.h"
#include "board.h"

void init_hash_keys();
void generate_position_key(Board* position);

#endif
