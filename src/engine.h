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
        Eval eval;
        int depth = 4;
};

class Enginev0 : public Engine{
    public:
        Enginev0(){}

        Enginev0(Board* _board, MoveGen* _movegen, int _depth){
            std::cout << "Searching to depth " << depth << std::endl;
            board = _board;
            movegen = _movegen;
            depth = _depth;

            Eval _eval(board, movegen);
            
            eval = _eval;
        }

        Move get_engine_move();
        
        int get_random_index(int moves_size);

        Move get_random_move();
};

class Enginev1 : public Engine{
    public:
        Enginev1(){}

        Enginev1(Board* _board, MoveGen* _movegen, int _depth){
            std::cout << "Searching to depth " << depth << std::endl;
            board = _board;
            movegen = _movegen;
            depth = _depth;

            Eval _eval(board, movegen);
            
            eval = _eval;
        }

        Move get_engine_move();

};

#endif