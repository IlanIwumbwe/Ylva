#ifndef THINK_H
#define THINK_H

#include "movegen.h"
#include "board.h"

typedef struct s_search_info{
    int nodes_searched;
    int maxdepth;

} search_info;

void think(search_info* info);

#endif


