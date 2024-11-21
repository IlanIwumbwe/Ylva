#ifndef THINK_H
#define THINK_H

#include "movegen.h"
#include "board.h"

typedef struct s_search_info{
    int nodes_searched;
    int maxdepth;
    U64 start_time;
    U64 end_time;
    int time_set;
    int stopped;
    board_state* state;
} search_info;

void think(search_info* info);

#endif


