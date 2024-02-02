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
        Eval();

        Eval(Board* _board, MoveGen* movegen);

        float PlainMinimax(int depth);

        float AlphaBetaMinimax(int depth, float alpha, float beta);

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
