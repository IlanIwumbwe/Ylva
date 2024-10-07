#ifndef BOARD_H
#define BOARD_H

#include "../headers/utils.h"
#include "../headers/array.h"

typedef struct sinfo{
    square ep_square;
    char castling_rights; /// only least significant 4 bits are used for castling rights
    int turn;  // 0 means white, 1 means black 
    int ply, moves;   
    U16 move; /// previous move that led to this state
    piece captured_piece; /// most recently captured piece
    // U64 occupied; /// bitboard of all occupied squares

} info;

extern info* board_info;
extern info board_infos[MAX_SEARCH_DEPTH];
extern U64 bitboards[12];
extern piece board[64]; /// which piece is on each square

void setup_state_from_fen(char* fen_string);

void setup_bitboards(const char* fen);

void print_board(void);

#endif