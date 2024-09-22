#ifndef BOARD_H
#define BOARD_H

#include "defs.h"
#include "move.h"
#include "zobrist.h"
#include <optional>
#include <assert.h>

struct State{
    uint8_t castling_rights;  
    int hm_clock;
    piece_names recent_capture;
    Move prev_move; 
    int ep_square;
    U64 hash_key;

    std::shared_ptr<State> prev_state = NULL;

    State(uint8_t cr, int hmc, piece_names rcap, Move prev, int ep_sq, U64 _hash_key, std::shared_ptr<State> _prev_state){
        castling_rights = cr;
        hm_clock = hmc;
        recent_capture = rcap;
        prev_move = prev;
        ep_square = ep_sq;
        hash_key = _hash_key;
        prev_state = _prev_state;
    }
};

class Board{
    public:
        Board ();

        ~Board(){free(pv_table.pv_entries);}

        void make_move(const Move& move);

        int get_prev_move(Move& prev_move);

        int undo_move();

        piece_names ep_capture(const colour& pawn_colour, const uint& to);

        void ep_uncapture(const colour& pawn_colour, const uint& to);

        void castle_kingside(const uint& king_square, const colour& king_colour);

        void uncastle_kingside(const uint& king_square, const colour& king_colour);

        void castle_queenside(const uint& king_square, const colour& king_colour);

        void uncastle_queenside(const uint& king_square, const colour& king_colour);
        
        void capture_piece(int square, const piece_names& to_piece_name);

        void uncapture_piece(const U64& square_bitboard, piece_names& recent_capture);

        bool has_castling_rights(int flag) const;

        bool is_occupied(uint square);

        piece_names get_promo_piece(const uint& flags, const colour& from_piece_colour);

        void init_from_fen(const std::string fen);

        void clear_bitboards(){
            for(int i = 0; i < 15; i++){
                bitboards[i] = 0ULL;
            }

            if(debug){std::cout << "info string cleared all bitboards" << std::endl;}
        }

        void init_board_state(const std::string& board_string);
        
        void init_turn(const std::string& str_turn);

        void init_castling_rights(const std::string& str_castling);

        Move init_enpassant_square(const std::string& str_ep_square);

        void init_halfmove_clock(const std::string& str_hm_clock);

        void init_fullmoves(const std::string& str_fullmoves);

        inline colour get_turn() const {return turn;}

        inline int get_hm_clock(){return hm_clock;}

        inline void change_turn(){turn = (colour)(1 - turn);}

        inline int get_ep_square()const{return ep_square;}

        inline u_int8_t get_castling_rights()const{return castling_rights & 0xf;}

        piece_names get_piece_on_square(uint square) const;

        void view_board();

        U64 get_entire_bitboard() const;

        void add_state(Move prev_move, piece_names recent_capture);

        void revert_state();

        void apply_psqt();

        void count_material();

        void add_to_eval(piece_names piece, int square);

        void sub_from_eval(piece_names piece, int square);

        // index 0 is black, index 1 is white
        int psqt_scores[2];
        int material[2];

        U64 hash_key = 0;

        // pv table
        PV_Table pv_table;
        uint16_t pv_array[MAX_DEPTH];

        int ply;

        // piece bitboards
        U64 bitboards[15];

    private:        
        // informaton from fen string
        colour turn;
        uint8_t castling_rights;          
        int hm_clock;
        int fullmoves;
        int ep_square;
        
        // maintain state
        std::shared_ptr<State> current_state = NULL;
};

void generate_position_key(Board* position);

void store_pv_move(Board* position, uint16_t move, int eval);

uint16_t probe_pv_move(Board* position);

std::optional<int> probe_pv_score(Board* position);


#endif