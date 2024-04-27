#ifndef ENGINE_H
#define ENGINE_H

#include "move.h"
#include "board.h"
#include "movegen.h"
#include "evaluation.h"
#include <random>
#include <chrono>

using namespace std::chrono;

#define PROMOTION_POWER 5
#define PAWN_ATTACK_POWER 2

class Engine{
    public:
        Engine(Board* _board, MoveGen* _movegen, int _depth) : board(_board), movegen(_movegen), depth(_depth), eval(board, movegen){}

        virtual Move get_engine_move() = 0;

        void engine_driver();
        
        /// Make move on the board, and increament nodes searched counter
        inline void make_move(Move move){
            board->make_move(move);    
            eval.nodes_searched += 1;
        }

        seconds time_used_per_turn = seconds(0);

    protected:
        Board* board;
        MoveGen* movegen;
        int depth = 4;
        Eval eval;
};

/// v0 has a plain minimax search algorithm
class Enginev0 : public Engine{
    public:
        Enginev0(Board* _board, MoveGen* _movegen, int _depth) : Engine(_board, _movegen, _depth) {}

        Move get_engine_move() override;

        int plain_minimax(int depth);
};

/// v1 has minimax optimised with alpha beta pruning
class Enginev1 : public Engine{
    public:
        Enginev1(Board* _board, MoveGen* _movegen, int _depth) : Engine(_board, _movegen, _depth) {}

        Move get_engine_move() override;

        int alpha_beta_minimax(int depth, int alpha, int beta);
};

/// @brief v2 has alpha beta minimax, and other optimisations
class Enginev2 : public Engine{
    public:
        Enginev2(Board* _board, MoveGen* _movegen, int _depth) : Engine(_board, _movegen, _depth) {}

        Move get_engine_move() override;

        int ab_move_ordering(int depth, int alpha, int beta);

        void pick_move(std::vector<Move>& moves, int start_index);

        void set_move_heuristics(std::vector<Move>& moves);

        int quiescence(int alpha, int beta);

        Move search_position(std::vector<Move>& moves, int search_depth);
};

#endif