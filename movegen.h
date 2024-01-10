#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "helper_funcs.h"
#include "board.h"
#include "move.h"

/// Given a board state, generate all valid moves in that state

class MoveGen{
    public:
        MoveGen(Board& current_state) : board(current_state){}

        void generate_moves(){
            // TODO
            whites = board.get_whites();
            blacks = board.get_blacks();
            occupied = board.get_entire_bitboard();

            std::cout << "Generating moves" << std::endl;
            board.set_valid_moves(moves);
        }

        void P_moves(){
            // forward 1
            if(board.get_turn() == WHITE){
                auto sqs = (whites << 8) & ~occupied & ~(RANK(8));        
            }
        }
        
    private:
        Board board;
        std::vector<Move> moves;
        uint64_t occupied, whites, blacks;

};

#endif