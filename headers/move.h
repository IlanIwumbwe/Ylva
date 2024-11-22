#ifndef MOVE_H
#define MOVE_H

#include "../headers/board.h"
#include "../headers/zobrist.h"

#define CAPTURE_MASK 0x4

void make_move(board_state* board, const U16 move);

void undo_move(board_state* state);

#endif