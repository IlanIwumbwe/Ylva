#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>
#include <limits.h>

#define U64 uint64_t
#define U16 uint16_t

#define get_lsb(bitboard) __builtin_ctzll(bitboard)
#define set_bit(sq) (1ULL << (sq))
#define move_from_square(move) ((move & 0xfc0) >> 6)
#define move_to_square(move) (move & 0x3f)
#define move_type(move) ((move & 0xf000) >> 12)
#define RANK(rank_num) ((uint64_t)0xff << (rank_num-1)*8)
#define RAND64 ((U64)(random() & 0xffff) | ((U64)(random() & 0xffff) << 16) | ((U64)(random() & 0xffff) << 32) | ((U64)(random() & 0xffff) << 48))
    
#define A_FILE 0x8080808080808080
#define B_FILE 0x4040404040404040
#define G_FILE 0x0202020202020202
#define H_FILE 0x0101010101010101   
#define K_castle 0x8
#define Q_castle 0x4
#define k_castle 0x2
#define q_castle 0x1
#define MAX_SEARCH_DEPTH 125
#define MAX_MOVES 218
#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

extern char char_pieces[12];
extern char* char_squares[64];

typedef enum {
    P,K,N,B,R,Q,p,k,n,b,r,q,p_none
} piece;

typedef enum {
    h1, g1, f1, e1, d1, c1, b1, a1, 
    h2, g2, f2, e2, d2, c2, b2, a2, 
    h3, g3, f3, e3, d3, c3, b3, a3, 
    h4, g4, f4, e4, d4, c4, b4, a4, 
    h5, g5, f5, e5, d5, c5, b5, a5, 
    h6, g6, f6, e6, d6, c6, b6, a6, 
    h7, g7, f7, e7, d7, c7, b7, a7, 
    h8, g8, f8, e8, d8, c8, b8, a8, 
    s_none,
} square;

typedef enum {WHITE, BLACK} side;

typedef enum {
    P_VAL = 100,
    K_VAL = 20000,
    N_VAL = 320,
    B_VAL = 330,
    R_VAL = 500,
    Q_VAL = 900
} vals;

extern vals piece_values[13]; 

typedef struct scpe{
    int kcr, qcr; // kingside and queenside castling rights (choose correct flags based on whose turn it is)
    int ep_sq_offset;
    piece rook_to_move, ep_pawn, ally_king;
    square rook_kingside_sq, rook_queenside_sq;
}castling_and_enpassant_info;

typedef struct smove {
    U16 move;
    int score;
} Move;

extern castling_and_enpassant_info cep_info[2];

piece char_to_piece(const char c_piece);

square char_to_square(const char* c_square);

piece piece_on_square(square sq);

void print_bitboard(const U64 number);

int count_set_bits(U64 number);

int tokenise(char* string, char* output[]);

void print_move(U16 move);

U16 move_from_str(char* move);

U64 time_in_ms();

int input_waiting();

int maxi(int a, int b);

int mini(int a, int b);

#endif
