#ifndef ENGINE_H
#define ENGINE_H

#include "move.h"
#include "board.h"
#include "movegen.h"
#include "zobrist.h"
#include <random>

#define PROMOTION_POWER 5
#define PAWN_ATTACK_POWER 2

class Engine{
    public:
        Engine(Board* _board, MoveGen* _movegen) : board(_board), movegen(_movegen){}

        /// @brief Get best move from a set of legal moves in that position
        /// @param legal_moves 
        virtual void get_engine_move(std::vector<Move>& legal_moves) = 0;

        void engine_driver(std::vector<Move>& legal_moves);
        
        /// Make move on the board, and increament nodes searched counter
        inline void make_move(Move move){
            board->make_move(move);    
            nodes_searched += 1;
        }
        
        /// @brief Get PV line given the legal moves that are in this position
        /// @param depth 
        /// @param legal_moves 
        /// @return 
        int get_pv_line(int depth);

        void set_depth(int d){depth = d;}

        void check_stop_conditions();

        void read_input();

        int evaluation(){
            int perspective = board->get_turn() ? 1 : -1;

            int eval = (board->material[1] - board->material[0]) + (board->psqt_scores[1] - board->psqt_scores[0]);

            return perspective * eval;
        }

        Board* board;
        MoveGen* movegen;

        U64 start_time, stop_time;
        bool stopped = false, time_set = false, quit = false;

        int pv_length = 0;

    protected:
        int depth, nodes_searched;  
        int pv_pointer = 0;
        Move best_move;
};

/// @brief v2 has alpha beta minimax, move ordering, and best move from previous pv line are searched first
class Enginev2 : public Engine{
    public:
        Enginev2(Board* _board, MoveGen* _movegen) : Engine(_board, _movegen) , killer_moves(max_killer_moves, std::vector<uint16_t>(MAX_DEPTH)){ }

        void get_engine_move(std::vector<Move>& legal_moves) override;

        int ab_search(int depth, int alpha, int beta);

        void set_move_heuristics(std::vector<Move>& moves);

        void store_killer_move(uint16_t current_move);

        int quiescence(int alpha, int beta);

        void search_position(std::vector<Move>& moves, int search_depth);

    private:
        int max_killer_moves = 2;
        std::vector<std::vector<uint16_t>> killer_moves;
};

void pick_move(std::vector<Move>& moves, int start_index);

bool move_exists(std::vector<Move>& legal_moves, Move move);

#endif