#ifndef EVAL_H
#define EVAL_H

#include "board.h"
#include "movegen.h"

#define CAPTURE_VAL_POWER 30
#define PROMOTION_POWER 50
#define PAWN_ATTACK_POWER 20

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
