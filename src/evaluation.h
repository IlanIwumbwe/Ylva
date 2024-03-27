#ifndef EVAL_H
#define EVAL_H

#include "board.h"
#include "movegen.h"

typedef enum{
    PAWN_VAL = 100,
    KNIGHT_VAL = 300,
    BISHOP_VAL = 300,
    ROOK_VAL = 500,
    QUEEN_VAL = 900
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

        inline void make_move(const Move& move){
            board->make_move(move);
            movegen->generate_moves();
        }

    private:
        Board* board;
        MoveGen* movegen;
};

#endif
