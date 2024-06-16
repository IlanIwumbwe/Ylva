#ifndef ENGINE_H
#define ENGINE_H

#include "move.h"
#include "board.h"
#include "movegen.h"
#include "evaluation.h"
#include "zobrist.h"
#include <random>

#define PROMOTION_POWER 5
#define PAWN_ATTACK_POWER 2

class Engine{
    public:
        Engine(Board* _board, MoveGen* _movegen) : board(_board), movegen(_movegen), eval(board, movegen){}

        /// @brief Get best move from a set of legal moves in that position
        /// @param legal_moves 
        virtual void get_engine_move(std::vector<Move>& legal_moves) = 0;

        void engine_driver(std::vector<Move>& legal_moves);
        
        /// Make move on the board, and increament nodes searched counter
        inline void make_move(Move move){
            board->make_move(move);    
            eval.nodes_searched += 1;
        }
        
        /// @brief Get PV line given the legal moves that are in this position
        /// @param depth 
        /// @param legal_moves 
        /// @return 
        int get_pv_line(int depth);

        void set_depth(int d){depth = d;}

        void check_stop_conditions();

        void read_input();

        Board* board;
        MoveGen* movegen;

        U64 start_time, stop_time;
        bool stopped = false, time_set = false, quit = false;

        int pv_length = 0;

    protected:
        int depth;  
        int pv_pointer = 0;
        Eval eval;
        Move best_move;
};

/// v0 has a plain minimax search algorithm
class Enginev0 : public Engine{
    public:
        Enginev0(Board* _board, MoveGen* _movegen) : Engine(_board, _movegen) {}

        void get_engine_move(std::vector<Move>& legal_moves) override;

        int plain_minimax(int depth);
};

/// v1 has minimax optimised with alpha beta pruning
class Enginev1 : public Engine{
    public:
        Enginev1(Board* _board, MoveGen* _movegen) : Engine(_board, _movegen) {}

        void get_engine_move(std::vector<Move>& legal_moves) override;

        int alpha_beta_minimax(int depth, int alpha, int beta);
};

/// @brief v2 has alpha beta minimax, move ordering, and best move from previous pv line are searched first
class Enginev2 : public Engine{
    public:
        Enginev2(Board* _board, MoveGen* _movegen) : Engine(_board, _movegen) {}

        void get_engine_move(std::vector<Move>& legal_moves) override;

        int ab_search(int depth, int alpha, int beta);

        void set_move_heuristics(std::vector<Move>& moves);

        int quiescence(int alpha, int beta);

        void search_position(std::vector<Move>& moves, int search_depth);
};

void pick_move(std::vector<Move>& moves, int start_index);

bool move_exists(std::vector<Move>& legal_moves, Move move);

void make_first(Move move, std::vector<Move>& moves);

#endif