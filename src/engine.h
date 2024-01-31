#ifndef ENGINE_H
#define ENGINE_H

#include "move.h"
#include "board.h"
#include "movegen.h"
#include <random>

class Engine{
    public:
        Engine(Board* board, MoveGen* movegen) : board(board), movegen(movegen){

        }

        int get_random_index(int moves_size){
            std::random_device rd;
            std::mt19937 gen(rd());

            std::uniform_int_distribution<> dist(0, moves_size-1);

            return dist(gen);
        }
        
        void make_engine_move(){
            board->view_board();   
            auto moves = board->get_valid_moves();
            auto move = moves[get_random_index(moves.size())];
            std::cout << move << std::endl;
            board->make_move(move);    
            movegen->generate_moves(); 
        }

    private:
        Board* board;
        MoveGen* movegen;
};

#endif