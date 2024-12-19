#ifndef PERFT_H
#define PERFT_H

#include "utils.h"
#include "board.h"
#include "movegen.h"
#include "move.h"

#ifdef DEV
U64 movegen_test(board_state* state, int depth);

void run_perft(board_state* state, int depth, int only_captures);

#endif

#endif