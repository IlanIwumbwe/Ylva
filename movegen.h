#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "helper_funcs.h"
#include "board.h"
#include "move.h"
#include <climits>

/// Given a board state, generate all valid moves in that state

class MoveGen{
    public:
        MoveGen(Board* current_state) : board(current_state){}

        void generate_moves(){
            std::cout << "Generating moves" << std::endl;

            // initialisations
            whites = board->get_whites();
            blacks = board->get_blacks();
            occupied = board->get_entire_bitboard();
            board->clear_valid_moves();
            turn = board->get_turn();

            attacked = 0;
            push_mask = ULLONG_MAX;
            capture_mask = ULLONG_MAX;

            P_moves();
            p_moves();

        }

        void P_moves(){
            auto white_pawns = board->get_piece_bitboard(P);
            
            // forward 1
            if(turn == WHITE){
                tos = (white_pawns << 8) & ~RANK(8) & ~occupied & push_mask;    
                add_valid_moves(tos,-8,0);
            }

            // forward 2
            if(turn == WHITE){                
                tos = ((white_pawns & RANK(2) & ~((occupied & RANK(3)) >> 8)) << 16) & ~occupied & push_mask;
                add_valid_moves(tos,-16,1);
            }

            // right captures
            tos = (white_pawns << 7) & ~RANK(8) & ~A_FILE & occupied & capture_mask;
            if(turn == WHITE){
                add_valid_moves(tos,-7,4); 
            } else {
                attacked |= tos;
            }


        }

        void p_moves(){
            auto black_pawns = board->get_piece_bitboard(p);

            // forward 1
            if(turn == BLACK){
                auto tos = (black_pawns >> 8) & ~occupied & ~RANK(1) & push_mask;    
                add_valid_moves(tos,8,0);
            }

            // forward 2
            if(turn == BLACK){                
                tos = ((black_pawns & RANK(7) & ~((occupied & RANK(6)) << 8)) >> 16) & ~occupied & push_mask;
                add_valid_moves(tos,16,1);
            }

            // right captures
            tos = (black_pawns >> 9) & ~RANK(1) & ~A_FILE & occupied & capture_mask;
            if(turn == BLACK){
                add_valid_moves(tos,9,4); 
            } else {
                attacked |= tos;
            }
        }

        /// Given a bitboard of destination squares, a pointer to the board state, an offset of calculate from square, and a flag to 
        /// indicate move type, add that move to the list of valid moves in board state
        void add_valid_moves(uint64_t& bitboard, int offset, unsigned int flag){
            while(bitboard){
                unsigned int to = __builtin_ctzll(bitboard);

                board->add_valid_move(Move(to+offset, to, flag));

                bitboard &= (bitboard-1);
            }
        }
        
    private:
        uint64_t tos;
        Board* board;
        uint64_t occupied, whites, blacks, push_mask, capture_mask, attacked;
        colour turn;

        // push mask sets all bits in positions on the board where we are allowed to move
        // capture mask sets all bits in positions on the board where we are allowed to capture

};

#endif