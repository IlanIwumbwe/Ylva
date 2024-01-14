#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "helper_funcs.h"
#include "board.h"
#include "move.h"
#include <climits>

/// Given a board state, generate all valid moves in that state
class MoveGen{
    public:
        MoveGen(Board* current_state) : board(current_state), prev_move(0,0,0) {}

        void generate_moves(){
            // initialisations
            whites = board->get_whites();
            blacks = board->get_blacks();

            white_pawns = board->get_piece_bitboard(P);
            black_pawns = board->get_piece_bitboard(p);
            white_king = board->get_piece_bitboard(K);
            black_king = board->get_piece_bitboard(k);

            occupied = whites | blacks;
            blacks_minus_king = blacks & ~black_king;
            whites_minus_king = whites & ~white_king;

            turn = board->get_turn();
            board->clear_valid_moves();

            get_legal_moves();
        }

        void get_legal_moves(){
            auto checkers_count = get_checkers();

            if(checkers_count <= 1){
                // modify push and capture mask if king is in check
                if(checkers_count == 1){
                    capture_mask = checkers;
                    set_push_mask(); 
                } else {
                    capture_mask = ULLONG_MAX;
                    push_mask = ULLONG_MAX;
                }

                if(turn == WHITE){
                    P_moves();
                    knight_moves(N);
                } else {
                    p_moves();
                    knight_moves(n);
                }
            }

            if(turn == WHITE){king_moves(K);}
            else {king_moves(k);}

            //printbitboard(get_attackers(1,BLACK));
            //std::cout << "Num of checkers " << checkers_count << std::endl; 
        }

        /// only call if in check by one piece
        /// If in check by a slider, push mask is squares between ally king and enemy slider
        /// If in check by non slider, push mask is 0
        void set_push_mask(){
            auto checker = board->get_piece_on_square(get_lsb(checkers));

            if(slider_piece(checker)){
                // TODO 
            } else {
                push_mask = 0;
            }
        }

        bool slider_piece(const piece_names& piece){
            return ((piece & 3) | (piece & 4) | (piece & 6)) != 0;
        }

        /// Given a king attack set, look through it and return a bitboard of those squares in the attack set that are attacked by enemy piece
        void set_king_danger_squares(uint64_t attack_set, int king_colour){
            king_danger_squares = 0;
            int lsb;

            while(attack_set){
                lsb = get_lsb(attack_set);

                if(get_attackers(lsb, ~king_colour)){
                    king_danger_squares |= lsb;
                }

                attack_set &= attack_set-1;
            }
        }

        /// Given a square, and a piece colour, return a bitboard of all pieces of that colour attacking that square
        U64 get_attackers(unsigned int square, const int colour){
            U64 out = 0;

            if(colour){
                out |= (knight_attack_set[square] & board->get_piece_bitboard(n));
                out |= set_bit(square+7) & ~A_FILE & black_pawns;
                out |= set_bit(square+9) & ~H_FILE & black_pawns;
                // TODO: rays from square to check for slider pieces giving check (white king should not be in blockers bitboard)
            } else {
                out |= (knight_attack_set[square] & board->get_piece_bitboard(N));
                out |= set_bit(square-9) & ~A_FILE & white_pawns;
                out |= set_bit(square-7) & ~H_FILE & white_pawns;
                // TODO: rays from square to check for slider pieces giving check (black king should not be in blockers bitboard)
            }

            return (out);
        }

        /// Produce bitboard of all pieces giving ally king check, and return the number of checkers
        unsigned int get_checkers(){
            unsigned int king_square = get_lsb((turn) ? black_king : white_king);
            checkers = get_attackers(king_square, ~turn);
            return count_set_bits(checkers);
        }

        void P_moves(){
            // forward 1
            tos = (white_pawns << 8) & ~RANK(8) & ~occupied & push_mask;   
            make_pawn_moves(tos,-8,0);

            // forward 2           
            tos = ((white_pawns & RANK(2) & ~((occupied & RANK(3)) >> 8)) << 16) & ~occupied & push_mask;
            make_pawn_moves(tos,-16,1);

            // right captures
            tos = (white_pawns << 7) & ~RANK(8) & ~A_FILE & blacks_minus_king & capture_mask;
            make_pawn_moves(tos,-7,4); 

            // left captures
            tos = (white_pawns << 9) & ~RANK(8) & ~H_FILE & blacks_minus_king & capture_mask;
            make_pawn_moves(tos,-9,4); 

            // promotion forward 1
            tos = (white_pawns << 8) & RANK(8) & ~occupied & push_mask;
            for(int i = 0; i < 4; ++i){
                make_pawn_moves(tos,-8,p_flags[i]);
            }

            // promotion right captures
            tos = (white_pawns << 7) & RANK(8) & ~A_FILE & blacks_minus_king & capture_mask;
            for(int i = 0; i < 4; ++i){
                make_pawn_moves(tos,-7,pc_flags[i]);
            }

            // promotion left captures
            tos = (white_pawns << 9) & RANK(8) & ~H_FILE & blacks_minus_king & capture_mask;
            for(int i = 0; i < 4; ++i){
                make_pawn_moves(tos,-9,pc_flags[i]);
            }

            // enpassant captures
            if(board->get_prev_move(prev_move) == 0 && (prev_move.get_flags() == 1)){
                pawn_bitboard = set_bit(prev_move.get_to());

                if((pawn_bitboard & whites) == 0){
                    // black pawn made the double pawn push, maybe we can capture it via enpassant
                    pawn_bitboard &= capture_mask | (push_mask >> 8);

                    // capture by white pawn to the right
                    tos = (white_pawns & (pawn_bitboard >> 1)) << 9;
                    make_pawn_moves(tos,-9,5);

                    // capture by white pawn to the left
                    tos = (white_pawns & (pawn_bitboard << 1)) << 7;
                    make_pawn_moves(tos,-7,5);
                }
            }      
        }

        void p_moves(){
            // forward 1
            tos = (black_pawns >> 8) & ~occupied & ~RANK(1) & push_mask;  
            make_pawn_moves(tos,8,0);
            
            // forward 2
            tos = ((black_pawns & RANK(7) & ~((occupied & RANK(6)) << 8)) >> 16) & ~occupied & push_mask;
            make_pawn_moves(tos,16,1);

            // right captures
            tos = (black_pawns >> 9) & ~RANK(1) & ~A_FILE & whites_minus_king & capture_mask;
            make_pawn_moves(tos,9,4); 

            // left captures
            tos = (black_pawns >> 7) & ~RANK(1) & ~H_FILE & whites_minus_king & capture_mask;
            make_pawn_moves(tos,7,4); 

            // promotion forward 1
            tos = (black_pawns >> 8) & RANK(1) & ~occupied & push_mask;
            for(int i = 0; i < 4; ++i){
                make_pawn_moves(tos,8,p_flags[i]);
            }

            // promotion right captures
            tos = (black_pawns >> 9) & RANK(1) & ~A_FILE & whites_minus_king & capture_mask;
            for(int i = 0; i < 4; ++i){
                make_pawn_moves(tos,9,pc_flags[i]); 
            }
            
            // promotion left captures
            tos = (black_pawns >> 7) & RANK(1) & ~H_FILE & whites_minus_king & capture_mask;
            for(int i = 0; i < 4; ++i){
                make_pawn_moves(tos,7,pc_flags[i]); 
            }

            // enpassant captures
            if(board->get_prev_move(prev_move) == 0 && (prev_move.get_flags() == 1)){
                pawn_bitboard = set_bit(prev_move.get_to());

                if((pawn_bitboard & blacks) == 0){
                    // white pawn made the double pawn push, maybe we can capture it via enpassant
                    pawn_bitboard &= capture_mask | (push_mask << 8);

                    // capture by black pawn to the right
                    tos = ((black_pawns & (pawn_bitboard >> 1)) >> 7);
                    make_pawn_moves(tos,7,5);

                    // capture by black pawn to the left
                    tos = (black_pawns & (pawn_bitboard << 1)) >> 9;
                    make_pawn_moves(tos,9,5);
                }

            }
        }

        void knight_moves(piece_names knight_name){
            auto knights = board->get_piece_bitboard(knight_name);
            U64 attack_set;
            unsigned int from;

            while(knights){
                from =  get_lsb(knights);
                attack_set = knight_attack_set[from];

                // tos for knight capture
                tos = attack_set & ((knight_name == N) ? blacks_minus_king : whites_minus_king) & capture_mask;
                make_other_moves(tos, from, 4);

                // tos for quiet knight move
                tos = attack_set & ~occupied & push_mask;
                make_other_moves(tos, from, 0);

                knights &= knights - 1;
            }
        }

        void king_moves(piece_names king_name){
            auto king = board->get_piece_bitboard(king_name);
            U64 attack_set, can_capture, can_push;
            unsigned int from;
            king_danger_squares = 0;

            from =  get_lsb(king);
            attack_set = king_attack_set[from];

            // filter out king danger squares
            set_king_danger_squares(attack_set, king_name > 7);

            can_capture = ((king_name == K) ? blacks_minus_king : whites_minus_king) & ~king_danger_squares;
            can_push = ~occupied & ~king_danger_squares;

            // tos for king capture
            tos = (attack_set & can_capture);
            make_other_moves(tos, from, 4);

            // tos for quiet king move
            tos = (attack_set & can_push);
            make_other_moves(tos, from, 0);

            if(king_name == K && board->has_castling_rights(K_castle)){
                if(!get_bit(occupied,2) && !get_bit(occupied,1) && !get_attackers(3,BLACK) && !get_attackers(2,BLACK) && !get_attackers(1,BLACK)){
                    make_other_moves(set_bit(1), 3, 2);
                }
            }

            if(king_name == k && board->has_castling_rights(k_castle)){
                if(!get_bit(occupied,58) && !get_bit(occupied,57) && !get_attackers(59,WHITE) && !get_attackers(58,WHITE) && !get_attackers(57,WHITE)){
                    make_other_moves(set_bit(57), 59, 2);
                }
            }

            if(king_name == K && board->has_castling_rights(Q_castle)){
                if(!get_bit(occupied,4) && !get_bit(occupied,5) && !get_bit(occupied,6) && !get_attackers(3,BLACK) && !get_attackers(4,BLACK) && !get_attackers(5,BLACK)){
                    make_other_moves(set_bit(5), 3, 3);
                }
            }

            if(king_name == k && board->has_castling_rights(q_castle)){
                if(!get_bit(occupied,60) && !get_bit(occupied,61) && !get_bit(occupied,62) && !get_attackers(59,WHITE) && !get_attackers(60,WHITE) && !get_attackers(61,WHITE)){
                    make_other_moves(set_bit(61), 59, 3);
                }
            }

        }


        /// Given a bitboard of destination squares, a pointer to the board state, an offset of calculate from square, and a flag to 
        /// indicate move type, add that move to the list of valid moves in board state
        void make_pawn_moves(U64 tos, int offset, unsigned int flag){
            unsigned int to;
            while(tos){
                to =  get_lsb(tos);

                board->add_valid_move(Move(to+offset, to, flag));

                tos &= tos-1;
            }
        }

        /// This receives the actual from square, as its the same for all tos that are passed to it
        void make_other_moves(U64 tos, unsigned int from, unsigned int flag){
            unsigned int to;
            while(tos){
                to =  get_lsb(tos);

                board->add_valid_move(Move(from, to, flag));

                tos &= tos-1;
            }
        }

    private:
        U64 tos;
        Board* board;
        U64 occupied, whites, blacks, whites_minus_king, blacks_minus_king;
        U64 king_danger_squares, checkers, push_mask, capture_mask;
        U64 white_pawns, black_pawns, white_king, black_king;
        colour turn;

        // needed for enpassant
        Move prev_move;
        U64 pawn_bitboard;

};

#endif