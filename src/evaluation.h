#ifndef EVAL_H
#define EVAL_H

#include "board.h"
#include "movegen.h"

#define CAPTURE_VAL_POWER 10
#define PROMOTION_POWER 20
#define PAWN_ATTACK_POWER 50ULL

class Eval{
    public:
        Eval();

        Eval(Board* _board, MoveGen* movegen);

        float plain_minimax(int depth);

        float alpha_beta_minimax(int depth, float alpha, float beta);

        int Evaluation();

        int count_black_material();

        int count_white_material();

        void set_move_heuristics(std::vector<Move>& moves, U64& enemy_pawns);

        void order_moves(std::vector<Move>& moves);

        void make_move(const Move& move){
            board->make_move(move);
            nodes_searched += 1;
            movegen->generate_moves();
        }

        int nodes_searched = 0;

    private:
        Board* board;
        MoveGen* movegen;

        std::unordered_map<piece_names, int> get_piece_value{
            {P, 100},
            {p, 100},
            {N, 300},
            {n, 300},
            {B, 300},
            {b, 300},
            {R, 500},
            {r, 500},
            {Q, 900},
            {q, 900}
        };
};

#endif
