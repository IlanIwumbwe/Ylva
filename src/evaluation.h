#ifndef EVAL_H
#define EVAL_H

#include "board.h"

typedef enum{
    PAWN = 100,
    KNIGHT = 300,
    BISHOP = 300,
    ROOK = 500,
    QUEEN = 900
} piece_vals;

int count_material(const Board* bo){
    auto turn = bo->get_turn();
    
    


}


#endif

