#ifndef THINK_H
#define THINK_H

#include "movegen.h"
#include "board.h"

#define PV_SCORE 2000000
#define CAPTURE_SCORE_OFFSET 1000000
#define KILLER_0_SCORE 9000000
#define KILLER_1_SCORE 8000000

typedef struct s_search_info{
    int nodes_searched;
    int maxdepth;
    U64 start_time;
    U64 end_time;
    int time_set;
    int stopped;
} search_info;

void think(search_info* info, board_state* state);

#endif


