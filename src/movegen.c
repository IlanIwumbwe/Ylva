#include "../headers/movegen.h"

int n_checkers;

U64 whites, blacks, occupied, whites_minus_king, blacks_minus_king, checkers, pinned_pieces, ally_king;

U64 KNIGHT_ATTACKS[64];
U64 KING_ATTACKS[64];
U64 PAWN_ATTACKS[64][2]; // for each square, bitboard of attacks from white / black's perspective

//  pregenerated moves for each square, for each configuration of blockers
U64 ROOK_MOVES[64][4096] = {};
U64 BISHOP_MOVES[64][4096] = {};

int rook_n_bits_in_span[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

int bishop_n_bits_in_span[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};

/// @brief magic numbers (hardcoded here after running the find magics function)
U64 rook_magic_nums[64] = {756607761056301088, 4917965982829391872, 9259436018514335872ULL, 252227969560612864, 1873504042192277572, 4683744712145502216,
        10736588108788171008ULL, 72060078754236160, 140880296050688, 36239972015016000, 288793611754082336, 10980338876383625296ULL, 281562084086016, 
        7219973898715005952, 288511855423520780, 703743277449344, 10430337286754283586ULL, 4616189892934762561, 845524712292416, 738733825827278848,
        81910317868647472, 10133648984637568, 27025995828578306, 24807181388087505, 4611826895502458880, 290517918687560456, 436853014294175744, 
        3891391591680573472, 8798240768384, 54608346652868736, 36037885170811138, 4621327094725559297, 18020034588704898, 794920529348804608, 
        38984971611803648, 4611765185420464128, 653444742557733889, 9379309380144470164ULL, 4616304006254301322, 5089704017996, 2305983773545627684, 
        1478940450466431008, 282162185437249, 36310615627333664, 145241122350858244, 563259460157452, 77124178800672772, 2533275872591874, 
        306325047602086400, 9876464420289249344ULL, 7206920489413936256, 5440992801384890624, 290490978518433920, 4400196747392, 9377629124748796928ULL, 
        4611734676179550720, 72093878194930306, 1299358895885811745, 3603162595299696641, 1424967491126529, 72339077738795013, 563518231807746, 4505833060814860, 4684308548237328530,};

U64 bishop_magic_nums[64] = {18050699908628544, 2490508190769291776, 730713440372408833, 2262812655092864, 4621258435378282593, 13793511079936000, 
        4901114932332331112, 285909564084224, 9243849418820686352ULL, 153695302212683808, 4839136495635923456, 649024400948986064, 12117148111554478592ULL, 
        218496668736160992, 4611831171218016385, 576461319843219072, 2886807636290962593, 36591815725942032, 10378563139621896208ULL, 1447942481952944128, 
        36311373927424034, 5699887612043264, 38844097754315776, 43989088993824, 3828077311429972224, 10957266947217555712ULL, 4683796390668403008, 
        436853562174083232, 9152334798004232, 4613104392873222272, 583286795383608834, 9241492263459161088ULL, 1164185176616736768, 36222346097361408, 
        13835102087354516000ULL, 3463270314627563649, 1153005084670443776, 3463270321061109888, 2315699038156227082, 1161676984475920, 74311060567384064, 
        144714426749290496, 1155262125015306240, 1441435838301865984, 4467445466114, 9117300746094080, 9078676108870146, 14556276145969430656ULL, 
        90287516691203088, 2414109724615656513, 4904492566345941762, 1227235297679900672, 9313728322179563528ULL, 618127948432025600, 18016614786170880, 
        577604295953490210, 9324739334098653696ULL, 290826329657472, 145242191801946128, 1735029486396904448, 211106501101058, 4579612160165920, 5136952819386432, 9011602301124914,};

magic_entry rook_magics[64];
magic_entry bishop_magics[64];

/// @brief Get all squares that the bishop spans when placed at this square. Edges of the board aren't included 
/// @param sq 
/// @return 
static U64 get_bishop_span(square sq){
    U64 attacks = 0ULL;
    int rank = sq % 8;
    int file = sq / 8;
    int r, f;

    for(r = rank-1, f = file-1; (r >= 1) && (f >= 1); r--, f--) attacks |= set_bit(8*f+r);
    for(r = rank+1, f = file+1; (r <= 6) && (f <= 6); r++, f++) attacks |= set_bit(8*f+r);
    for(r = rank+1, f = file-1; (r <= 6) && (f >= 1); r++, f--) attacks |= set_bit(8*f+r);
    for(r = rank-1, f = file+1; (r >= 1) && (f <= 6); r--, f++) attacks |= set_bit(8*f+r);

    return attacks;
}

/// @brief Get all squares that the rook spans when placed at this square. Edges of the board aren't included 
/// @param sq 
/// @return 
static U64 get_rook_span(square sq){
    U64 attacks = 0ULL;
    int rank = sq % 8;
    int file = sq / 8;
    int r, f;

    for(r = rank-1; r >= 1; r--) attacks |= set_bit(8*file+r);
    for(r = rank+1; r <= 6; r++) attacks |= set_bit(8*file+r);
    for(f = file-1; f >= 1; f--) attacks |= set_bit(8*f+rank);
    for(f = file+1; f <= 6; f++) attacks |= set_bit(8*f+rank);

    return attacks;
}

static U64 get_bishop_attacks(square sq, U64 blockers){
    U64 sq_bb, attacks = 0ULL;
    int rank = sq % 8;
    int file = sq / 8;
    int r, f;

    for(r = rank-1, f = file-1; (r >= 0) && (f >= 0); r--, f--) {
        sq_bb = set_bit(8*f+r);
        attacks |= sq_bb;

        if(sq_bb & blockers) break;
    }
    for(r = rank+1, f = file+1; (r <= 7) && (f <= 7); r++, f++) {
        sq_bb = set_bit(8*f+r);
        attacks |= sq_bb;

        if(sq_bb & blockers) break;
    }
    for(r = rank+1, f = file-1; (r <= 7) && (f >= 0); r++, f--) {
        sq_bb = set_bit(8*f+r);
        attacks |= sq_bb;

        if(sq_bb & blockers) break;
    }
    for(r = rank-1, f = file+1; (r >= 0) && (f <= 7); r--, f++) {
        sq_bb = set_bit(8*f+r);
        attacks |= sq_bb;

        if(sq_bb & blockers) break;
    }

    return attacks;
}

static U64 get_rook_attacks(square sq, U64 blockers){
    U64 sq_bb, attacks = 0ULL;
    int rank = sq % 8;
    int file = sq / 8;
    int r, f;

    for(r = rank-1; r >= 0; r--) {
        sq_bb = set_bit(8*file+r);
        attacks |= sq_bb;

        if(sq_bb & blockers) break;
    }
    for(r = rank+1; r <= 7; r++) {
        sq_bb = set_bit(8*file+r);
        attacks |= sq_bb;

        if(sq_bb & blockers) break;
    }
    for(f = file-1; f >= 0; f--) {
        sq_bb = set_bit(8*f+rank);
        attacks |= sq_bb;

        if(sq_bb & blockers) break;
    }
    for(f = file+1; f <= 7; f++) {
        sq_bb = set_bit(8*f+rank);
        attacks |= sq_bb;

        if(sq_bb & blockers) break;
    }

    return attacks;
}

static U64 get_blocker_configuration(int number, magic_entry mge){
    square sq;
    U64 out = 0ULL, span = mge.span;

    for(int i = 0; i < mge.n_bits_in_span; ++i){
        sq = get_lsb(span);
        span &= (span - 1);

        if(number & set_bit(i)){out |= set_bit(sq);}
    }

    return out;
}

static int blocker_to_key(U64 blocker, magic_entry mge){
    return ((blocker & mge.span) * mge.magic_num) >> (64 - mge.n_bits_in_span);
}

/// @brief find magic number for this square that can produce distinct indices for all blocker configurations
/// @return magic number
static U64 find_magic(square sq, int for_bishop){

    U64 magic, attack_masks[4096], blocker_masks[4096], table[4096];
    magic_entry mge;

    if(for_bishop){
        mge.span = get_bishop_span(sq);
        mge.n_bits_in_span = bishop_n_bits_in_span[sq]; 
    } else {
        mge.span = get_rook_span(sq);
        mge.n_bits_in_span = rook_n_bits_in_span[sq];
    }

    // 2**n_bits_in_span tells you the total number of configurations of blocker pieces when a slider is at that square

    // setup each blocker configuration in the blocker_masks array
    // setup the attack masks given the blocker configuration i

    int j;

    for(j = 0; j < (1 << mge.n_bits_in_span); ++j){
        blocker_masks[j] = get_blocker_configuration(j, mge);   
        attack_masks[j] = (for_bishop) ? get_bishop_attacks(sq, blocker_masks[j]) : get_rook_attacks(sq, blocker_masks[j]);
    }

    // try to fill up table for this square such that there are no collisions
    int fail;
    int key;

    for(size_t i = 0; i < 100000000; ++i){

        magic = RAND64 & RAND64 & RAND64;
        mge.magic_num = magic;

        for(j = 0; j < 4096; ++j){table[j] = 0ULL;}

        for(j = 0, fail = 0; !fail && (j < (1 << mge.n_bits_in_span)); ++j){
            key = blocker_to_key(blocker_masks[j], mge);

            if(table[key] == 0ULL){table[key] = attack_masks[j];}
            else if(table[key] != attack_masks[j]){fail = 1;}
        }

        if(!fail) return magic;
    }

    return 0ULL;
}

void find_all_magics(){
    U64 magic;

    printf("\rooks\n");

    // rook magics
    for(int i = 0; i < 64; ++i){
        magic = find_magic(i, 0);
        printf("%lu, ", magic);
    }

    printf("\nbishops\n");

    for(int i = 0; i < 64; ++i){
        magic = find_magic(i, 1);
        printf("%lu, ", magic);
    }
}

/// Produce bitboard of all attacked squares by a knight at given square
static void knight_attacks(square sq){
    U64 bitboard = set_bit(sq);
    
    KNIGHT_ATTACKS[sq] |= (bitboard & ~(G_FILE | H_FILE | RANK(8))) << 6;
    KNIGHT_ATTACKS[sq] |= (bitboard & ~(RANK(7) | RANK(8) | H_FILE)) << 15;
    KNIGHT_ATTACKS[sq] |= (bitboard & ~(A_FILE | B_FILE | RANK(8))) << 10;
    KNIGHT_ATTACKS[sq] |= (bitboard & ~(A_FILE | RANK(7) | RANK(8))) << 17;
    KNIGHT_ATTACKS[sq] |= (bitboard & ~(G_FILE | H_FILE | RANK(1))) >> 10;
    KNIGHT_ATTACKS[sq] |= (bitboard & ~(H_FILE | RANK(1) | RANK(2))) >> 17;
    KNIGHT_ATTACKS[sq] |= (bitboard & ~(A_FILE | B_FILE | RANK(1))) >> 6;
    KNIGHT_ATTACKS[sq] |= (bitboard & ~(A_FILE | RANK(1) | RANK(2))) >> 15;
}

/// Produce bitboard of all attacked squares by a king at given square
static void king_attacks(square sq){
    U64 bitboard = set_bit(sq);
    
    KING_ATTACKS[sq] |= (bitboard & ~RANK(8)) << 8;
    KING_ATTACKS[sq] |= (bitboard & ~H_FILE) >> 1;
    KING_ATTACKS[sq] |= (bitboard & ~A_FILE) << 1;
    KING_ATTACKS[sq] |= (bitboard & ~RANK(1)) >> 8;
    KING_ATTACKS[sq] |= (bitboard & ~(RANK(8) | H_FILE)) << 7;
    KING_ATTACKS[sq] |= (bitboard & ~(RANK(8) | A_FILE)) << 9;
    KING_ATTACKS[sq] |= (bitboard & ~(RANK(1) | H_FILE)) >> 9;
    KING_ATTACKS[sq] |= (bitboard & ~(RANK(1) | A_FILE)) >> 7;
}

static void white_pawn_attacks(square sq){
    U64 bitboard = set_bit(sq);

    // right captures
    PAWN_ATTACKS[sq][WHITE] |= (bitboard & ~(RANK(8) | H_FILE)) << 7;

    // left captures
    PAWN_ATTACKS[sq][WHITE] |= (bitboard & ~(RANK(8) | A_FILE)) << 9; 
}

static void black_pawn_attacks(square sq){
    U64 bitboard = set_bit(sq);

    // right captures
    PAWN_ATTACKS[sq][BLACK] |= (bitboard & ~(RANK(1) | H_FILE)) >> 9;

    // left captures
    PAWN_ATTACKS[sq][BLACK] |= (bitboard & ~(RANK(1) | A_FILE)) >> 7; 
}

static void pregenerate_slider_moves(){
    U64 blocker_config;
    magic_entry mge;
    int key, i, j, n_bits_in_span;

    for(i = 0; i < 64; ++i){

        // setup magic entries
        mge.magic_num = bishop_magic_nums[i];
        mge.span = get_bishop_span(i);
        mge.n_bits_in_span = bishop_n_bits_in_span[i];
        bishop_magics[i] = mge;

        // bishop moves
        mge = bishop_magics[i];
        n_bits_in_span = bishop_n_bits_in_span[i];

        for(j = 0; j < (1 << n_bits_in_span); ++j){
            blocker_config = get_blocker_configuration(j, mge);
            key = blocker_to_key(blocker_config, mge);
            BISHOP_MOVES[i][key] = get_bishop_attacks(i, blocker_config);
        }

        // setup magic entries
        mge.magic_num = rook_magic_nums[i];
        mge.span = get_rook_span(i);
        mge.n_bits_in_span = rook_n_bits_in_span[i];
        rook_magics[i] = mge;

        // rook moves
        mge = rook_magics[i];
        n_bits_in_span = rook_n_bits_in_span[i];

        for(j = 0; j < (1 << n_bits_in_span); ++j){
            blocker_config = get_blocker_configuration(j, mge);
            key = blocker_to_key(blocker_config, mge);
            ROOK_MOVES[i][key] = get_rook_attacks(i, blocker_config);
        }
    }

}

void populate_attack_sets(){
    for(int i = 0; i < 64; ++i){
        knight_attacks(i);
        king_attacks(i);
        white_pawn_attacks(i);
        black_pawn_attacks(i);
    }

    // use stored magic numbers to generate attack sets for the slider pieces
    pregenerate_slider_moves();
}

static void create_pawn_moves(dynamic_array* moves_array, U64 tos, int offset, int flag){
    square to;
    Move m;

    while(tos){
        to = get_lsb(tos);

        m.move = (flag << 12) | ((to+offset) << 6) | to;

        da_append(moves_array, m);

        tos &= tos-1;
    }
}

static void create_other_moves(dynamic_array* moves_array, U64 tos, square from, int flag){
    square to;
    Move m;

    while(tos){
        to = get_lsb(tos);

        m.move = (flag << 12) | (from << 6) | to;

        da_append(moves_array, m);

        tos &= tos-1;   
    }
}

/// @brief Return a bitboard of all pieces attacking the square
/// @param sq 
/// @param mask attackers that are in this bitboard will not be considered
/// @return 
static U64 get_attackers(square sq, U64 mask){
    U64 result;
    int key;
    U64 bishops, rooks;

    result = KNIGHT_ATTACKS[sq] & (bitboards[N] | bitboards[n]);
    result |= KING_ATTACKS[sq] & (bitboards[K] | bitboards[k]);

    key = blocker_to_key(board_info->occupied , bishop_magics[sq]);
    bishops = BISHOP_MOVES[sq][key];

    key = blocker_to_key(board_info->occupied, rook_magics[sq]);    
    rooks = ROOK_MOVES[sq][key];

    result |= bishops & (bitboards[B] | bitboards[b]); 
    result |= rooks & (bitboards[R] | bitboards[r]);
    result |= (bishops | rooks) & (bitboards[Q] | bitboards[q]);

    result |= (PAWN_ATTACKS[sq][WHITE] & bitboards[p]) | (PAWN_ATTACKS[sq][BLACK] & bitboards[P]);
    
    return result & ~mask;
}

/// @brief Make each pseudo legal move, if it leaves the enemy king in check, then it is illegal. Also frees memory used by the pseudo-legal array
/// @param psuedo_legal_moves 
/// @param moves_array 
/// @param ally_king
/// @param ally_pieces
static void filter_pseudo_legal_moves(dynamic_array* pseudo_legal_moves, dynamic_array* moves_array, piece ally_pawn, piece ally_king, piece ally_knight, piece ally_bishop, piece ally_rook, piece ally_queen){
    size_t i;
    Move move;
    square sq;
    U64 attackers;

    for(i = 0; i < pseudo_legal_moves->used; ++i){
        move = pseudo_legal_moves->array[i];

        make_move(move.move);

        sq = get_lsb(bitboards[ally_king]);

        attackers = get_attackers(sq, bitboards[ally_pawn] | bitboards[ally_king] | bitboards[ally_knight] | bitboards[ally_bishop] | bitboards[ally_rook] | bitboards[ally_queen]);

        if(!attackers){da_append(moves_array, move);}

        undo_move();
    }
}

static void K_quiet_moves(dynamic_array* moves_array){

    square sq = get_lsb(bitboards[K]);

    U64 move_set = KING_ATTACKS[sq] & ~occupied;

    create_other_moves(moves_array, move_set, sq, 0);

    if(n_checkers == 0){
        if(board_info->castling_rights & K_castle){
            if(!(set_bit(f1) & occupied) && !(set_bit(g1) & occupied) && !get_attackers(f1,whites) && !get_attackers(g1,whites)){
                create_other_moves(moves_array, set_bit(g1), e1, 2);
            }
        }

        if(board_info->castling_rights & Q_castle){
            if(!(set_bit(d1) & occupied) && !(set_bit(c1) & occupied) && !(set_bit(b1) & occupied) && !get_attackers(d1,whites) && !get_attackers(c1,whites)){
                create_other_moves(moves_array, set_bit(c1), e1, 3);
            }
        }
    }
}

static void K_captures_moves(dynamic_array* moves_array){

    square sq = get_lsb(bitboards[K]);

    U64 move_set = KING_ATTACKS[sq] & blacks;

    create_other_moves(moves_array, move_set, sq, 4);
}

static void k_quiet_moves(dynamic_array* moves_array){

    square sq = get_lsb(bitboards[k]);

    U64 move_set = KING_ATTACKS[sq] & ~occupied;

    create_other_moves(moves_array, move_set, sq, 0);

    if(n_checkers == 0){
        if(board_info->castling_rights & k_castle){
            if(!(set_bit(f8) & occupied) && !(set_bit(g8) & occupied) && !get_attackers(f8,blacks) && !get_attackers(g8,blacks)){
                create_other_moves(moves_array, set_bit(g8), e8, 2);
            }
        } else {
            board_info->castling_rights &= ~k_castle;
        }

        if(board_info->castling_rights & q_castle){
            if(!(set_bit(d8) & occupied) && !(set_bit(c8) & occupied) && !(set_bit(b8) & occupied) && !get_attackers(d8,blacks) && !get_attackers(c8,blacks)){
                create_other_moves(moves_array, set_bit(c8), e8, 3);
            }
        } else {
            board_info->castling_rights &= ~q_castle;
        }
    }
}

static void k_captures_moves(dynamic_array* moves_array){

    square sq = get_lsb(bitboards[k]);

    U64 move_set = KING_ATTACKS[sq] & whites;

    create_other_moves(moves_array, move_set, sq, 4);

}

static void N_quiet_moves(dynamic_array* moves_array){ 
    U64 white_knights = bitboards[N];
    U64 tos;
    square sq;

    while(white_knights){
        sq = get_lsb(white_knights);
        white_knights &= (white_knights - 1);

        tos = KNIGHT_ATTACKS[sq] & ~occupied;

        create_other_moves(moves_array,tos,sq,0);
    }
}

static void N_captures_moves(dynamic_array* moves_array){ 
    U64 white_knights = bitboards[N];
    U64 tos;
    square sq;

    while(white_knights){
        sq = get_lsb(white_knights);
        white_knights &= (white_knights - 1);

        tos = KNIGHT_ATTACKS[sq] & blacks_minus_king;

        create_other_moves(moves_array,tos,sq,4);
    }
}

static void n_quiet_moves(dynamic_array* moves_array){ 
    U64 black_knights = bitboards[n];
    U64 tos;
    square sq;

    while(black_knights){
        sq = get_lsb(black_knights);
        black_knights &= (black_knights - 1);

        tos = KNIGHT_ATTACKS[sq] & ~occupied;

        create_other_moves(moves_array,tos,sq,0);
    }
}

static void n_captures_moves(dynamic_array* moves_array){ 
    U64 black_knights = bitboards[n];
    U64 tos;
    square sq;

    while(black_knights){
        sq = get_lsb(black_knights);
        black_knights &= (black_knights - 1);

        tos = KNIGHT_ATTACKS[sq] & whites_minus_king;

        create_other_moves(moves_array,tos,sq,4);
    }
}

static void P_quiet_moves(dynamic_array* moves_array){
    U64 white_pawns = bitboards[P];
    U64 tos;

    // forward 1
    tos = (white_pawns << 8) & ~RANK(8) & ~occupied;   
    create_pawn_moves(moves_array, tos,-8,0);

    // forward 2           
    tos = ((white_pawns & RANK(2) & ~((occupied & RANK(3)) >> 8)) << 16) & ~occupied;
    create_pawn_moves(moves_array, tos,-16,1);

    // promotion forward 1
    tos = (white_pawns << 8) & RANK(8) & ~occupied;

    for(int i = 0; i < 4; ++i){
        create_pawn_moves(moves_array,tos,-8,i+8);
    }  
}

static void P_captures_moves(dynamic_array* moves_array){
    U64 white_pawns = bitboards[P], doubly_pushed_pawn;
    U64 tos;
    int prev_move;

    // right captures
    tos = (white_pawns << 7) & ~RANK(8) & ~A_FILE & blacks_minus_king;
    create_pawn_moves(moves_array,tos,-7,4); 

    // left captures
    tos = (white_pawns << 9) & ~RANK(8) & ~H_FILE & blacks_minus_king;
    create_pawn_moves(moves_array,tos,-9,4); 

    // promotion right captures
    tos = (white_pawns << 7) & RANK(8) & ~A_FILE & blacks_minus_king;
    for(int i = 0; i < 4; ++i){
        create_pawn_moves(moves_array,tos,-7, i+12);
    }

    // promotion left captures
    tos = (white_pawns << 9) & RANK(8) & ~H_FILE & blacks_minus_king;
    for(int i = 0; i < 4; ++i){
        create_pawn_moves(moves_array,tos,-9, i+12);
    }

    // enpassant captures
    prev_move = board_info->move;

    if(move_type(prev_move) == 1){
        doubly_pushed_pawn = set_bit(move_to_square(prev_move));

        if(!(doubly_pushed_pawn & whites)){
            // black pawn made the double pawn push, maybe we can capture it via enpassant
           
            // capture by white pawn to the right
            tos = (white_pawns & ((doubly_pushed_pawn & ~H_FILE) >> 1)) << 9;
            create_pawn_moves(moves_array,tos,-9,5);

            // capture by white pawn to the left
            tos = (white_pawns & ((doubly_pushed_pawn & ~A_FILE) << 1)) << 7;
            create_pawn_moves(moves_array,tos,-7,5);
        }
    }  
}

static void p_quiet_moves(dynamic_array* moves_array){
    U64 black_pawns = bitboards[p];
    U64 tos;

    // forward 1
    tos = (black_pawns >> 8) & ~occupied & ~RANK(1);  
    create_pawn_moves(moves_array,tos,8,0);
    
    // forward 2
    tos = ((black_pawns & RANK(7) & ~((occupied & RANK(6)) << 8)) >> 16) & ~occupied;
    create_pawn_moves(moves_array,tos,16,1);

    // promotion forward 1
    tos = (black_pawns >> 8) & RANK(1) & ~occupied;
    for(int i = 0; i < 4; ++i){
        create_pawn_moves(moves_array,tos,8,i+8);
    }
}

static void p_captures_moves(dynamic_array* moves_array){
    U64 black_pawns = bitboards[p], doubly_pushed_pawn; 
    U64 tos;
    int prev_move;

    // right captures
    tos = (black_pawns >> 9) & ~RANK(1) & ~A_FILE & whites_minus_king;
    create_pawn_moves(moves_array,tos,9,4); 

    // left captures
    tos = (black_pawns >> 7) & ~RANK(1) & ~H_FILE & whites_minus_king;
    create_pawn_moves(moves_array,tos,7,4); 

    // promotion right captures
    tos = (black_pawns >> 9) & RANK(1) & ~A_FILE & whites_minus_king;
    for(int i = 0; i < 4; ++i){
        create_pawn_moves(moves_array,tos,9,i+12); 
    }
    
    // promotion left captures
    tos = (black_pawns >> 7) & RANK(1) & ~H_FILE & whites_minus_king;
    for(int i = 0; i < 4; ++i){
        create_pawn_moves(moves_array,tos,7,i+12); 
    }

    // enpassant captures
    prev_move = board_info->move;

    if(move_type(prev_move) == 1){
        doubly_pushed_pawn = set_bit(move_to_square(prev_move));

        if(!(doubly_pushed_pawn & blacks)){
            // white pawn made the double pawn push, maybe we can capture it via enpassant
           
            // capture by black pawn to the right
            tos = (black_pawns & ((doubly_pushed_pawn & ~H_FILE) >> 1)) >> 7;
            create_pawn_moves(moves_array,tos,7,5);

            // capture by white pawn to the left
            tos = (black_pawns & ((doubly_pushed_pawn & ~A_FILE) << 1)) >> 9;
            create_pawn_moves(moves_array,tos,9,5);
        }
    } 
}

static void B_captures_moves(dynamic_array* moves_array){
    U64 white_bishops = bitboards[B];
    U64 tos;
    int key;
    square sq;

    while(white_bishops){
        sq = get_lsb(white_bishops);
        white_bishops &= (white_bishops-1);

        key = blocker_to_key(occupied, bishop_magics[sq]);
        tos = (BISHOP_MOVES[sq][key] & blacks_minus_king);

        create_other_moves(moves_array, tos, sq, 4);
    }
}

static void B_quiet_moves(dynamic_array* moves_array){
    U64 white_bishops = bitboards[B];
    U64 tos;
    int key;
    square sq;

    while(white_bishops){
        sq = get_lsb(white_bishops);
        white_bishops &= (white_bishops-1);

        key = blocker_to_key(occupied, bishop_magics[sq]);
        tos = (BISHOP_MOVES[sq][key] & ~occupied);

        create_other_moves(moves_array, tos, sq, 0);
    }
}

static void b_captures_moves(dynamic_array* moves_array){
    U64 black_bishops = bitboards[b];
    U64 tos;
    int key;
    square sq;

    while(black_bishops){
        sq = get_lsb(black_bishops);
        black_bishops &= (black_bishops-1);

        key = blocker_to_key(occupied, bishop_magics[sq]);
        tos = (BISHOP_MOVES[sq][key] & whites_minus_king);

        create_other_moves(moves_array, tos, sq, 4);
    }
}

static void b_quiet_moves(dynamic_array* moves_array){
    U64 black_bishops = bitboards[b];
    U64 tos;
    int key;
    square sq;

    while(black_bishops){
        sq = get_lsb(black_bishops);
        black_bishops &= (black_bishops-1);

        key = blocker_to_key(occupied, bishop_magics[sq]);
        tos = (BISHOP_MOVES[sq][key] & ~occupied);

        create_other_moves(moves_array, tos, sq, 0);
    }
}

static void R_captures_moves(dynamic_array* moves_array){
    U64 white_rooks = bitboards[R];
    U64 tos;
    int key;
    square sq;

    while(white_rooks){
        sq = get_lsb(white_rooks);
        white_rooks &= (white_rooks-1);

        key = blocker_to_key(occupied, rook_magics[sq]);
        tos = (ROOK_MOVES[sq][key] & blacks_minus_king);

        create_other_moves(moves_array, tos, sq, 4);
    }
}

static void R_quiet_moves(dynamic_array* moves_array){
    U64 white_rooks = bitboards[R];
    U64 tos;
    int key;
    square sq;

    while(white_rooks){
        sq = get_lsb(white_rooks);
        white_rooks &= (white_rooks-1);

        key = blocker_to_key(occupied, rook_magics[sq]);
        tos = (ROOK_MOVES[sq][key] & ~occupied);

        create_other_moves(moves_array, tos, sq, 0);
    }
}

static void r_captures_moves(dynamic_array* moves_array){
    U64 black_rooks = bitboards[r];
    U64 tos;
    int key;
    square sq;

    while(black_rooks){
        sq = get_lsb(black_rooks);
        black_rooks &= (black_rooks-1);

        key = blocker_to_key(occupied, rook_magics[sq]);
        tos = (ROOK_MOVES[sq][key] & whites_minus_king);

        create_other_moves(moves_array, tos, sq, 4);
    }
}

static void r_quiet_moves(dynamic_array* moves_array){
    U64 black_rooks = bitboards[r];
    U64 tos;
    int key;
    square sq;

    while(black_rooks){
        sq = get_lsb(black_rooks);
        black_rooks &= (black_rooks-1);

        key = blocker_to_key(occupied, rook_magics[sq]);
        tos = (ROOK_MOVES[sq][key] & ~occupied);

        create_other_moves(moves_array, tos, sq, 0);
    }
}

static void Q_captures_moves(dynamic_array* moves_array){
    U64 white_queens = bitboards[Q];

    U64 tos;
    int key;
    square sq;

    while(white_queens){
        sq = get_lsb(white_queens);
        white_queens &= (white_queens-1);

        key = blocker_to_key(occupied, rook_magics[sq]);
        tos = ROOK_MOVES[sq][key];

        key = blocker_to_key(occupied, bishop_magics[sq]);
        tos |= BISHOP_MOVES[sq][key];
        
        tos &= blacks_minus_king;

        create_other_moves(moves_array, tos, sq, 4);
    }
}

static void Q_quiet_moves(dynamic_array* moves_array){
    U64 white_queens = bitboards[Q];

    U64 tos;
    int key;
    square sq;

    while(white_queens){
        sq = get_lsb(white_queens);
        white_queens &= (white_queens-1);

        key = blocker_to_key(occupied, rook_magics[sq]);
        tos = ROOK_MOVES[sq][key];

        key = blocker_to_key(occupied, bishop_magics[sq]);
        tos |= BISHOP_MOVES[sq][key];
        
        tos &= ~occupied;

        create_other_moves(moves_array, tos, sq, 0);
    }
}

static void q_captures_moves(dynamic_array* moves_array){
    U64 black_queens = bitboards[q];

    U64 tos;
    int key;
    square sq;

    while(black_queens){
        sq = get_lsb(black_queens);
        black_queens &= (black_queens-1);

        key = blocker_to_key(occupied, rook_magics[sq]);
        tos = ROOK_MOVES[sq][key];

        key = blocker_to_key(occupied, bishop_magics[sq]);
        tos |= BISHOP_MOVES[sq][key];
        
        tos &= whites_minus_king;

        create_other_moves(moves_array, tos, sq, 4);
    }
}

static void q_quiet_moves(dynamic_array* moves_array){
    U64 black_queens = bitboards[q];

    U64 tos;
    int key;
    square sq;

    while(black_queens){
        sq = get_lsb(black_queens);
        black_queens &= (black_queens-1);

        key = blocker_to_key(occupied, rook_magics[sq]);
        tos = ROOK_MOVES[sq][key];

        key = blocker_to_key(occupied, bishop_magics[sq]);
        tos |= BISHOP_MOVES[sq][key];
        
        tos &= ~occupied;

        create_other_moves(moves_array, tos, sq, 0);
    }
}

void generate_moves(dynamic_array* moves_array, int captures_only){

    whites = bitboards[P] | bitboards[K] | bitboards[N] | bitboards[B] | bitboards[R] | bitboards[Q];
    blacks = bitboards[p] | bitboards[k] | bitboards[n] | bitboards[b] | bitboards[r] | bitboards[q];

    whites_minus_king = whites & ~bitboards[K];
    blacks_minus_king = blacks & ~bitboards[k];

    occupied = board_info->occupied;

    //assert((whites | blacks) == occupied);

    dynamic_array pseudo_legal_moves;
    init_da(&pseudo_legal_moves, 218);

    if(board_info->s == BLACK){
        checkers = get_attackers(get_lsb(bitboards[k]), blacks);
        n_checkers = count_set_bits(checkers);

        // generate king moves
        k_captures_moves(&pseudo_legal_moves);
        if(!captures_only){k_quiet_moves(&pseudo_legal_moves);}

        // generate moves for other pieces, as there may be evasions if n_checkers is 1. If n_checkers is 0, all legal moves are valid
        if(n_checkers <= 1){
            
            p_captures_moves(&pseudo_legal_moves);
            n_captures_moves(&pseudo_legal_moves);
            r_captures_moves(&pseudo_legal_moves);
            b_captures_moves(&pseudo_legal_moves);
            q_captures_moves(&pseudo_legal_moves);

            if(!captures_only){
                p_quiet_moves(&pseudo_legal_moves);
                n_quiet_moves(&pseudo_legal_moves);
                r_quiet_moves(&pseudo_legal_moves);
                b_quiet_moves(&pseudo_legal_moves);
                q_quiet_moves(&pseudo_legal_moves);
            }

        } 

        filter_pseudo_legal_moves(&pseudo_legal_moves, moves_array, p, k, n, b, r, q);

    } else {
        checkers = get_attackers(get_lsb(bitboards[K]), whites);
        n_checkers = count_set_bits(checkers);

        // generate king moves
        K_captures_moves(&pseudo_legal_moves);
        if(!captures_only){K_quiet_moves(&pseudo_legal_moves);}

        // generate moves for other pieces, as there may be evasions if n_checkers is 1. If n_checkers is 0, all legal moves are valid
        if(n_checkers <= 1){
            
            P_captures_moves(&pseudo_legal_moves);
            N_captures_moves(&pseudo_legal_moves);
            R_captures_moves(&pseudo_legal_moves);
            B_captures_moves(&pseudo_legal_moves);
            Q_captures_moves(&pseudo_legal_moves);

            if(!captures_only){
                P_quiet_moves(&pseudo_legal_moves);
                N_quiet_moves(&pseudo_legal_moves);
                R_quiet_moves(&pseudo_legal_moves);
                B_quiet_moves(&pseudo_legal_moves);
                Q_quiet_moves(&pseudo_legal_moves);
            }

        } 

        filter_pseudo_legal_moves(&pseudo_legal_moves, moves_array, P, K, N, B, R, Q);
    }

    free_da(&pseudo_legal_moves);
}