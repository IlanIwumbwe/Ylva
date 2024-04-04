#ifndef ENGINE_H
#define ENGINE_H

#include "move.h"
#include "board.h"
#include "movegen.h"
#include "evaluation.h"
#include <random>
#include <chrono>

using namespace std::chrono;

class Engine{
    public:
        Engine(Board* _board, MoveGen* _movegen, int _depth) : board(_board), movegen(_movegen), depth(_depth), eval(board){
            std::cout << "Searching to depth " << depth << std::endl;
        }

        virtual Move get_engine_move() = 0;

        void engine_driver();

        inline void make_move(Move move){
            board->make_move(move);    
            eval.nodes_searched += 1;
            movegen->generate_moves(); 
        }

    protected:
        Board* board;
        MoveGen* movegen;
        int depth = 4;
        Eval eval;
};

class Enginev0 : public Engine{
    public:
        Enginev0(Board* _board, MoveGen* _movegen, int _depth) : Engine(_board, _movegen, _depth) {}

        Move get_engine_move() override;

        float plain_minimax(int depth);
};

class Enginev1 : public Engine{
    public:
        Enginev1(Board* _board, MoveGen* _movegen, int _depth) : Engine(_board, _movegen, _depth) {}

        Move get_engine_move() override;

        float alpha_beta_minimax(int depth, float alpha, float beta);
};

class Enginev2 : public Engine{
    public:
        Enginev2(Board* _board, MoveGen* _movegen, int _depth) : Engine(_board, _movegen, _depth) {}

        Move get_engine_move() override;

        float ab_move_ordering(int depth, float alpha, float beta);

        void set_move_heuristics(std::vector<Move>& moves, U64& enemy_pawns);

        void order_moves(std::vector<Move>& moves);
};

#endif