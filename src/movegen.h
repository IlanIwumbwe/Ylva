#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "board.h"
#include "move.h"
#include <climits>

/// Given a board state, generate all valid moves in that state
class MoveGen{
    public:
        MoveGen(Board* current_state);

        std::vector<Move> generate_moves(bool _captures_only = false);

        bool move_is_legal(Move& move);

        std::vector<Move> get_legal_moves();

        bool no_legal_moves();

        /// generate all legal moves
        void generate_legal_moves();

        /// generate only captures moves
        void generate_legal_captures();

        /// Setup a bitboard of all pinned pieces on the board. This mask is used to remove pinned pieces when such that moves aren't generated for them
        /// in main move generator. Instead, pinned pieces' moves are generated separately at right after king moves are generated
        void get_pinned_pieces();

        void pins_along_diag(U64& potential_pin, uint& pinner_sq, const uint& ally_king_sq);

        void pins_along_nondiag(U64& potential_pin, uint& pinner_sq, const uint& ally_king_sq);

        bool valid_slider_pin(piece_names& pinned_piece_name, ray_type _ray_type);

        /// Generate all valid moved for pinned pieces on the board
        void pinned_moves(uint& pinner_sq, U64& possible_pin, const U64& pinned_bitboard, ray_type _ray_type);

        /// only call if in check by one piece
        /// If in check by a slider, push mask is squares between ally king and enemy slider
        /// If in check by non slider, push mask is 0
        void set_push_mask();

        U64 mask_opposing_rays(uint sq1, uint sq2, int start, int end);

        bool slider_piece(const piece_names& piece);

        bool is_bishop(const piece_names& piece);

        bool is_rook(const piece_names& piece);

        bool is_queen(const piece_names& piece);

        /// Given a king attack set, look through it and return a bitboard of those squares in the attack set that are attacked by enemy piece
        void set_king_danger_squares(U64 attack_set, int king_colour);

        U64 get_attackers(uint square, const int colour);

        void get_pawn_attackers(U64& pawn_attackers, uint square, const int colour);

        /// Produce bitboard of all pieces giving ally king check, and return the number of checkers
        uint get_checkers();

        inline bool ally_king_in_check(){
            return checkers != 0;
        }

        void P_quiet_moves();

        void P_captures_moves();

        void p_quiet_moves();

        void p_captures_moves();

        bool ep_discovered_check(U64 ep_mask, U64 enemy_rooks, U64 enemy_queens);

        void N_quiet_moves();

        void N_captures_moves();

        void n_quiet_moves();

        void n_captures_moves();

        void K_quiet_moves();

        void K_captures_moves();

        void k_quiet_moves();

        void k_captures_moves();

        void R_quiet_moves();

        void R_captures_moves();

        void r_quiet_moves();

        void r_captures_moves();

        void B_quiet_moves();

        void B_captures_moves();

        void b_quiet_moves();

        void b_captures_moves();

        void Q_quiet_moves();

        void Q_captures_moves();

        void q_quiet_moves();

        void q_captures_moves();

        /// Given a bitboard of destination squares, a pointer to the board state, an offset of calculate from square, and a flag to 
        /// indicate move type, add that move to the list of valid moves in board state
        void create_pawn_moves(U64 tos, int offset, uint flag);

        /// This receives the actual from square, as its the same for all tos that are passed to it
        void create_other_moves(U64 tos, uint from, uint flag);

        U64 get_positive_ray_attacks(U64 occupied, dirs dir, uint square);

        U64 get_negative_ray_attacks(U64 occupied, dirs dir, uint square);

        inline U64 get_file_attacks(U64 occupied, uint square){
            return get_positive_ray_attacks(occupied, north, square) | get_negative_ray_attacks(occupied, south, square);
        }

        inline U64 get_rank_attacks(U64 occupied, uint square){
            return get_positive_ray_attacks(occupied, west, square) | get_negative_ray_attacks(occupied, east, square);
        }

        inline U64 get_diagonal_attacks(U64 occupied, uint square){
            return get_positive_ray_attacks(occupied, noEa, square) | get_negative_ray_attacks(occupied, soWe, square);
        }

        inline U64 get_antidiagonal_attacks(U64 occupied, uint square){
            return get_positive_ray_attacks(occupied, noWe, square) | get_negative_ray_attacks(occupied, soEa, square);
        }

        inline U64 get_rook_attacks(U64 occupied, uint square){
            return get_file_attacks(occupied, square) | get_rank_attacks(occupied, square);
        }

        inline U64 get_bishop_attacks(U64 occupied, uint square){
            return get_diagonal_attacks(occupied, square) | get_antidiagonal_attacks(occupied, square);
        }

        inline U64 get_queen_attacks(U64 occupied, uint square){
            return get_rook_attacks(occupied, square) | get_bishop_attacks(occupied, square);
        }

    private:
        U64 tos;
        Board* board;
        U64 occupied, whites, blacks, whites_minus_king, blacks_minus_king;
        U64 king_danger_squares, checkers, push_mask, capture_mask;
        U64 white_pawns, black_pawns, white_king, black_king, ally_king, ally_pieces;
        U64 white_bishops, black_bishops, white_queens, black_queens, white_rooks, black_rooks, white_knights, black_knights;
        U64 diag_pinners, nondiag_pinners, pinned_pieces = 0;
        colour turn;

        // needed for enpassant
        Move prev_move;
        U64 pawn_bitboard;

        int checkers_count;
        bool captures_only = false;

        std::vector<Move> legal_moves;
};

#endif