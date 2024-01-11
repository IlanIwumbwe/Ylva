#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "helper_funcs.h"
#include "board.h"
#include "move.h"

/// Given a board state, generate all valid moves in that state
class MoveGen{
    public:
        MoveGen(Board* current_state) : board(current_state), prev_move(0,0,0) {}

        void generate_moves(){
            std::cout << "Generating moves" << std::endl;

            // initialisations
            whites = board->get_whites();
            blacks = board->get_blacks();
            occupied = board->get_entire_bitboard();
            occupied_minus_kings = board->get_capturable_bitboard();

            white_pawns = board->get_piece_bitboard(P);
            black_pawns = board->get_piece_bitboard(p);
            white_king = board->get_piece_bitboard(K);
            black_king = board->get_piece_bitboard(k);

            turn = board->get_turn();
            board->clear_valid_moves();

            // generate bitboard of king danger squares, and bitboard of checkers
            get_checkers();
            // TODO: get_king_danger_squares()

            printbitboard(checkers);

            if(turn == WHITE){
                P_moves();
            } else {
                p_moves();
            }
        }

        /// Produce bitboard of all pieces giving ally king check
        void get_checkers(){
            unsigned int king_square;
            checkers = 0;

            if(turn == WHITE){
                king_square = __builtin_ctzll(white_king);

                checkers |= (knight_attack_set[king_square] & board->get_piece_bitboard(n));
                checkers |= (white_king << 7) & ~A_FILE & black_pawns;
                checkers |= (white_king << 9) & ~H_FILE & black_pawns;
                // TODO: rays from king square to check for slider pieces giving check
            } else {
                king_square = __builtin_ctzll(black_king);

                checkers |= (knight_attack_set[king_square] & board->get_piece_bitboard(N));
                checkers |= (black_king >> 9) & ~A_FILE & white_pawns;
                checkers |= (white_king >> 7) & ~H_FILE & white_pawns;
                // TODO: rays from king square to check for slider pieces giving check
            }
        }

        void P_moves(){
            
            // forward 1
            tos = (white_pawns << 8) & ~RANK(8) & ~occupied;   
            add_valid_moves(tos,-8,0);

            // forward 2           
            tos = ((white_pawns & RANK(2) & ~((occupied & RANK(3)) >> 8)) << 16) & ~occupied;
            add_valid_moves(tos,-16,1);

            // right captures
            tos = (white_pawns << 7) & ~RANK(8) & ~A_FILE & blacks & occupied_minus_kings;
            add_valid_moves(tos,-7,4); 

            // left captures
            tos = (white_pawns << 9) & ~RANK(8) & ~H_FILE & blacks & occupied_minus_kings;
            add_valid_moves(tos,-9,4); 

            // promotion forward 1
            tos = (white_pawns << 8) & RANK(8) & ~occupied;
            for(int i = 0; i < 4; ++i){
                add_valid_moves(tos,-8,p_flags[i]);
            }

            // promotion right captures
            tos = (white_pawns << 7) & RANK(8) & ~A_FILE & blacks & occupied_minus_kings;
            for(int i = 0; i < 4; ++i){
                add_valid_moves(tos,-7,pc_flags[i]);
            }

            // promotion left captures
            tos = (white_pawns << 9) & RANK(8) & ~H_FILE & blacks & occupied_minus_kings;
            for(int i = 0; i < 4; ++i){
                add_valid_moves(tos,-9,pc_flags[i]);
            }

            // enpassant captures
            if(board->get_prev_move(prev_move) == 0 && (prev_move.get_flags() == 1)){
                pawn_bitboard = (1ULL << prev_move.get_to());

                if((pawn_bitboard & whites) == 0){
                    // black pawn made the double pawn push, maybe we can capture it via enpassant
                    
                    // capture by white pawn to the right
                    tos = (white_pawns & (pawn_bitboard >> 1)) << 9;
                    add_valid_moves(tos,-9,5);

                    // capture by white pawn to the left
                    tos = (white_pawns & (pawn_bitboard << 1)) << 7;
                    add_valid_moves(tos,-7,5);
                }
            }   
            
        }

        void p_moves(){
            // forward 1
            auto tos = (black_pawns >> 8) & ~occupied & ~RANK(1);  
            add_valid_moves(tos,8,0);
            
            // forward 2
            tos = ((black_pawns & RANK(7) & ~((occupied & RANK(6)) << 8)) >> 16) & ~occupied;
            add_valid_moves(tos,16,1);

            // right captures
            tos = (black_pawns >> 9) & ~RANK(1) & ~A_FILE & whites & occupied_minus_kings;
            add_valid_moves(tos,9,4); 

            // left captures
            tos = (black_pawns >> 7) & ~RANK(1) & ~H_FILE & whites & occupied_minus_kings;
            add_valid_moves(tos,7,4); 

            // promotion forward 1
            tos = (black_pawns >> 8) & RANK(1) & ~occupied;
            for(int i = 0; i < 4; ++i){
                add_valid_moves(tos,8,p_flags[i]);
            }

            // promotion right captures
            tos = (black_pawns >> 9) & RANK(1) & ~A_FILE & whites & occupied_minus_kings;
            for(int i = 0; i < 4; ++i){
                add_valid_moves(tos,9,pc_flags[i]); 
            }
            
            // promotion left captures
            tos = (black_pawns >> 7) & RANK(1) & ~H_FILE & whites & occupied_minus_kings;
            for(int i = 0; i < 4; ++i){
                add_valid_moves(tos,7,pc_flags[i]); 
            }

            // enpassant captures
            if(board->get_prev_move(prev_move) == 0 && (prev_move.get_flags() == 1)){
                pawn_bitboard = (1ULL << prev_move.get_to());

                if((pawn_bitboard & blacks) == 0){
                    // white pawn made the double pawn push, maybe we can capture it via enpassant

                    // capture by black pawn to the right
                    tos = (black_pawns & (pawn_bitboard >> 1)) >> 7;
                    add_valid_moves(tos,7,5);

                    // capture by black pawn to the left
                    tos = (black_pawns & (pawn_bitboard << 1)) >> 9;
                    add_valid_moves(tos,9,5);
                }

            }
        }

        /// Given a bitboard of destination squares, a pointer to the board state, an offset of calculate from square, and a flag to 
        /// indicate move type, add that move to the list of valid moves in board state
        void add_valid_moves(uint64_t bitboard, int offset, unsigned int flag){
            while(bitboard){
                unsigned int to = __builtin_ctzll(bitboard);

                board->add_valid_move(Move(to+offset, to, flag));

                bitboard &= (bitboard-1);
            }
        }
        
    private:
        uint64_t tos;
        Board* board;
        uint64_t occupied, whites, blacks, king_danger_squares, occupied_minus_kings, checkers;
        uint64_t white_pawns, black_pawns, white_king, black_king;
        colour turn;

        // needed for enpassant
        Move prev_move;
        uint64_t pawn_bitboard;

};

#endif