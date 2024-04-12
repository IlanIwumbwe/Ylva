#ifndef EVAL_H
#define EVAL_H

#include "board.h"
#include "movegen.h"

#define CAPTURE_VAL_POWER 3
#define PROMOTION_POWER 5
#define PAWN_ATTACK_POWER 2

class Eval{
    public:
        Eval();

        Eval(Board* _board);

        int Evaluation();

        int count_black_material();

        int count_white_material();

        int nodes_searched = 0;

    private:
        Board* board;
};

#endif
