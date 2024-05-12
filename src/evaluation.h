#ifndef EVAL_H
#define EVAL_H

#include "board.h"
#include "movegen.h"

class Eval{
    public:
        Eval();

        Eval(Board* _board, MoveGen* _movegen);

        int Evaluation();

        int count_black_material();

        int count_white_material();

        int king_movement_endgame_eval();

        int nodes_searched = 0;

    private:
        Board* board;
        MoveGen* movegen;
};

#endif
