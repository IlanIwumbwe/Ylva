#ifndef ENGINE_H
#define ENGINE_H

#include "move.h"
#include "board.h"
#include "movegen.h"
#include <random>
class Engine{
    public:
        Engine(Board* board, MoveGen* movegen);

        int get_random_index(int moves_size);

        Move get_best_move();

        Move get_random_move();
        
        void make_engine_move();

    private:
        Board* board;
        MoveGen* movegen;
};

#endif