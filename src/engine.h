#ifndef ENGINE_H
#define ENGINE_H

#include "move.h"
#include "board.h"
#include "movegen.h"
#include "evaluation.h"
#include <random>

class Engine{
    public:
        Engine(Board* board, MoveGen* movegen, int depth);

        int get_random_index(int moves_size);

        Move find_minimax_move();

        Move get_random_move();
        
        void make_engine_move();

        inline void make_move(Move move){
            board->make_move(move);    
            movegen->generate_moves(); 
        }

    private:
        Board* board;
        MoveGen* movegen;
        Eval eval;
        int depth = 4;
};

#endif