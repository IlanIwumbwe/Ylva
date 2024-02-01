#ifndef BOARD_H
#define BOARD_H

#include "defs.h"
#include "move.h"
#include <assert.h>

struct State{
    uint8_t castling_rights;  
    int hm_clock;
    //std::vector<Move> valid_moves;
    piece_names recent_capture;
    Move prev_move; 
    std::shared_ptr<State> prev_state = NULL;

    static int state_id;

    State(uint8_t cr, int hmc, piece_names rcap, Move prev){
        castling_rights = cr;
        hm_clock = hmc;
        recent_capture = rcap;
        prev_move = prev;

        state_id++;
    }
};

class Board{
    public:
        Board (const std::string& _fen);

        /// Make move given as input on the board
        void make_move(const Move& move);

        int get_prev_move(Move& prev_move);

        int undo_move();

        /// Perform en-passant capture
        piece_names ep_capture(const colour& pawn_colour, const uint& to);

        /// Revert en-passant capture
        void ep_uncapture(const colour& pawn_colour, const uint& to);

        void castle_kingside(const uint& king_square, const colour& king_colour);

        void uncastle_kingside(const uint& king_square, const colour& king_colour);

        void castle_queenside(const uint& king_square, const colour& king_colour);

        void uncastle_queenside(const uint& king_square, const colour& king_colour);
        
        /// Perform normal capture
        void capture_piece(const piece_names& to_piece_name, const U64& square_bitboard);

        /// Revert normal capture
        void uncapture_piece(const U64& square_bitboard, piece_names& recent_capture);

        bool has_castling_rights(int flag) const;

        bool is_occupied(uint square);

        piece_names get_promo_piece(const uint& flags, const colour& from_piece_colour);

        void init_from_fen(const std::vector<std::string>& parts);

        /// Initialise all bitboards and piece lists
        void init_board_state(const std::string& board_string);
        
        void init_turn(const std::string& str_turn);

        void init_castling_rights(const std::string& str_castling);

        /// Given the enpassant square, add the relevant move to move history that would've led to that enpassant square, which will be used to 
        /// validate possible enpassant captures by an enemy piece
        Move init_enpassant_square(const std::string& str_ep_square);

        void init_halfmove_clock(const std::string& str_hm_clock);

        void init_fullmoves(const std::string& str_fullmoves);

        int get_fullmoves();

        inline colour get_turn() const {return turn;}

        inline int get_hm_clock(){return hm_clock;}

        inline void change_turn(){turn = (colour)~turn;}

        piece_names get_piece_on_square(int square) const;

        void view_board();

        bool move_is_valid(Move& move);

        void clear_valid_moves();

        void set_piece_bitboard(const piece_names& piece_name, const U64& bitboard);

        colour get_piece_colour(const piece_names& piece_name);

        inline U64 get_piece_bitboard(const piece_names& piece_name) {
            assert(piece_name != None);
            return bitboards[piece_name];
        }

        U64 get_entire_bitboard() const;

        void add_valid_move(const Move& valid_move);

        std::vector<Move> get_valid_moves();

        /// after a new move has been made, create a new state. Store the new castling rights, the half move clock, the move that led to this state,
        /// and the piece if any that got captured when that move was made
        void add_state(Move prev_move, piece_names recent_capture);

        /// Make the previous state the current state, then make valid moves the valid moves of that previous state
        void revert_state();

    private:        
        // informaton from fen string
        colour turn;
        uint8_t castling_rights;          
        int hm_clock;
        int fullmoves;
        int ep_square;

        // piece bitboards
        std::unordered_map<piece_names, U64> bitboards{};
        //U64 pawns[2];
        //U64 knights[2];
        //U64 bishops[2];
        //U64 queens[2];
        //U64 kings[2];
        //U64 rooks[2];
        
        // maintain state
        std::shared_ptr<State> current_state = NULL;

        std::vector<Move> valid_moves;

};

#endif
