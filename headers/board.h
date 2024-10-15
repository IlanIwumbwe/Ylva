#ifndef BOARD_H
#define BOARD_H

#include "../headers/utils.h"
#include "../headers/array.h"

typedef struct sinfo{
    square ep_square;
    char castling_rights; /// only least significant 4 bits are used for castling rights
    side s;  // 0 means white, 1 means black 
    int ply, moves, hisply;   
    U16 move; /// previous move that led to this state
    piece captured_piece; /// most recently captured piece
    U64 hash;
    U64 occupied;
    int material[2];
} info;

extern info* board_info;
extern info board_infos[MAX_SEARCH_DEPTH];

extern U64 bitboards[13]; // last bitboard is a garbage bitboard
extern piece board[64]; /// which piece is on each square

extern U64 piece_zobrist_keys[13][64];
extern U64 turn_key;
extern U64 castling_key[16];

extern U16 pv_array[MAX_SEARCH_DEPTH];
extern pv_table pvt;

void setup_state_from_fen(const char* fen_string);

void setup_bitboards(const char* fen);

void print_board(void);

void init_hash_keys(void);

void modify_hash_by_occupancy(info* info_n, piece p, square sq);

void modify_hash_by_castling_rights(info* info_n, U16 old_castling_rights);

void generate_hash();

void count_material();

#endif