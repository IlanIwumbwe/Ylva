#ifndef BOARD_H
#define BOARD_H

#include "../headers/utils.h"
#include "../headers/array.h"

typedef struct shistory{
    square ep_square;
    char castling_rights; /// only least significant 4 bits are used for castling rights
    side s;  // 0 means white, 1 means black 
    int ply, moves, hisply;
    int fifty_move;   
    U16 move; /// previous move that led to this state
    piece captured_piece; /// most recently captured piece
    U64 hash;
    U64 occupied;
    int eval[2];
} history;

/// @brief Principal variation entry in PV table stores best move assosiated with each position
typedef struct spve{
    U64 key;
    U16 move;
} pv_entry;

typedef struct spvt{
    pv_entry* table;
    size_t capacity;
} pv_table;

typedef struct sboard {
    U64 bitboards[13];
    piece board[64];
    history metadata[MAX_SEARCH_DEPTH + 10];
    history* data;
    pv_table pvt;
    U16 pv_array[MAX_SEARCH_DEPTH + 10];
    U64 checkers;
    int n_checkers;

    U16 history_moves[64][64];
    U16 killer_moves[2][MAX_SEARCH_DEPTH];
} board_state;

void setup_state_from_fen(board_state* state, const char* fen_string);

void setup_bitboards(board_state* state, const char* fen);

void print_board(const board_state* state);

void init_eval(board_state* state);

void init_pv(board_state* state , size_t capacity);

void reset_pv_entries(board_state* state);

void store_pv_entry(board_state* state, U16 move);

U16 probe_pv_table(board_state* state);

void free_pv(pv_table* pvt);

void clear_pv_array(U16* array);

#endif