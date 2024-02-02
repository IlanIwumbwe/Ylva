#ifndef EVAL_H
#define EVAL_H

#include "board.h"
#include "movegen.h"

typedef enum{
    PAWN = 100,
    KNIGHT = 300,
    BISHOP = 300,
    ROOK = 500,
    QUEEN = 900
} piece_vals;

class Eval{
    public:
        Eval(Board* _board, MoveGen* movegen);

        int PlainMinimax(int depth);

        int Evaluation();

        int count_black_material();

        int count_white_material();

        inline void make_move(Move move){
            board->make_move(move);
            movegen->generate_moves();
        }

    private:
        Board* board;
        MoveGen* movegen;
};

#endif
