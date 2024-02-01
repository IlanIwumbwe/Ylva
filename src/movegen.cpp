#include "movegen.h"

/// Given a board state, generate all valid moves in that state
MoveGen::MoveGen(Board* current_state) : board(current_state), prev_move(0,0,0) {
}

void MoveGen::generate_moves(){
    // initialisations
    white_pawns = board->get_piece_bitboard(P);
    black_pawns = board->get_piece_bitboard(p);
    white_king = board->get_piece_bitboard(K);
    black_king = board->get_piece_bitboard(k);
    white_knights = board->get_piece_bitboard(N);
    black_knights = board->get_piece_bitboard(n);

    white_bishops = board->get_piece_bitboard(B);
    white_queens = board->get_piece_bitboard(Q);
    white_rooks = board->get_piece_bitboard(R);

    black_bishops = board->get_piece_bitboard(b);
    black_queens = board->get_piece_bitboard(q);
    black_rooks = board->get_piece_bitboard(r);

    whites = white_pawns | white_king | white_knights | white_bishops | white_queens | white_rooks;
    blacks = black_pawns | black_king | black_knights | black_bishops | black_queens | black_rooks;

    occupied = whites | blacks;
    blacks_minus_king = blacks & ~black_king;
    whites_minus_king = whites & ~white_king;

    turn = board->get_turn();
    board->clear_valid_moves();
    pinned_pieces = 0;
    
    get_legal_moves();         
}

void MoveGen::get_legal_moves(){

    if(turn == WHITE){
        ally_king = white_king;
        ally_pieces = whites_minus_king;
        diag_pinners = black_bishops | black_queens;
        nondiag_pinners = black_rooks | black_queens;

        K_moves();
    } else {
        ally_king = black_king;
        ally_pieces = blacks_minus_king;
        diag_pinners = white_bishops | white_queens;
        nondiag_pinners = white_rooks | white_queens;

        k_moves();
    }

    if(checkers_count <= 1){
        get_pinned_pieces();

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
            N_moves();
            R_moves();
            B_moves();
            Q_moves();
        } else {
            p_moves();
            n_moves();
            r_moves();
            b_moves();
            q_moves();
        }
    }
}

/// Setup a bitboard of all pinned pieces on the board. This mask is used to remove pinned pieces when such that moves aren't generated for them
/// in main move generator. Instead, pinned pieces' moves are generated separately at right after king moves are generated
void MoveGen::get_pinned_pieces(){ 
    // TODO
    uint ally_king_sq = get_lsb(ally_king), pinner_sq;
    U64 potential_pin;

    pins_along_diag(potential_pin, pinner_sq, ally_king_sq);
    pins_along_nondiag(potential_pin, pinner_sq, ally_king_sq);
}

void MoveGen::pins_along_diag(U64& potential_pin, uint& pinner_sq, const uint& ally_king_sq){
    U64 pinned_piece;

    while(diag_pinners){
        pinner_sq = get_lsb(diag_pinners);
        potential_pin = mask_opposing_rays(pinner_sq, ally_king_sq, 4, 7);

        pinned_piece = potential_pin & ally_pieces;

        if((pinned_piece != 0) && (count_set_bits(potential_pin & occupied) == 1)){
            if(checkers_count == 0){pinned_moves(pinner_sq, potential_pin, pinned_piece, diag);}
            pinned_pieces |= pinned_piece;
        }
        
        diag_pinners &= diag_pinners - 1;
    }
}

void MoveGen::pins_along_nondiag(U64& potential_pin, uint& pinner_sq, const uint& ally_king_sq){
    U64 pinned_piece;

    while(nondiag_pinners){
        pinner_sq = get_lsb(nondiag_pinners);
        potential_pin = mask_opposing_rays(pinner_sq, ally_king_sq, 0, 3);

        pinned_piece = potential_pin & ally_pieces;

        if((pinned_piece != 0) && (count_set_bits(potential_pin & occupied) == 1)){
            if(checkers_count == 0){pinned_moves(pinner_sq, potential_pin, pinned_piece, nondiag);}
            pinned_pieces |= pinned_piece;
        }
        
        nondiag_pinners &= nondiag_pinners - 1;
    }
}

bool MoveGen::valid_slider_pin(piece_names& pinned_piece_name, ray_type _ray_type){
    if(is_queen(pinned_piece_name)){
        return true;
    } else {
        return (is_bishop(pinned_piece_name) && (_ray_type == diag)) || (is_rook(pinned_piece_name) && (_ray_type == nondiag));
    }
}

/// Generate all valid moved for pinned pieces on the board
void MoveGen::pinned_moves(uint& pinner_sq, U64& possible_pin, const U64& pinned_bitboard, ray_type _ray_type){
    auto pinned_sq = get_lsb(pinned_bitboard);
    auto pinned_piece_name = board->get_piece_on_square(pinned_sq);

    if(valid_slider_pin(pinned_piece_name, _ray_type)){
        create_other_moves(possible_pin & ~pinned_bitboard, pinned_sq, 0);
        create_other_moves(set_bit(pinner_sq), pinned_sq, 4);
    } else if (pinned_piece_name == P){
        // quiet moves
        tos = (pinned_bitboard << 8) & possible_pin;
        create_pawn_moves(tos, -8, 0);
        
        tos = ((pinned_bitboard & RANK(2)) << 16) & possible_pin;
        create_pawn_moves(tos, -16, 1);

        // right / left captures
        tos = (pinned_bitboard << 7) & set_bit(pinner_sq);
        create_pawn_moves(tos,-7,4);

        tos = (pinned_bitboard << 9) & set_bit(pinner_sq);
        create_pawn_moves(tos,-9,4);

    } else if(pinned_piece_name == p){
        // quiet moves
        tos = (pinned_bitboard >> 8) & possible_pin;
        create_pawn_moves(tos, 8, 0);
        
        tos = ((pinned_bitboard & RANK(7)) >> 16) & possible_pin;
        create_pawn_moves(tos, 16, 1);

        // right / left captures
        tos = (pinned_bitboard >> 9) & set_bit(pinner_sq);
        create_pawn_moves(tos,9,4);

        tos = (pinned_bitboard >> 7) & set_bit(pinner_sq);
        create_pawn_moves(tos,7,4);
    }
}

/// only call if in check by one piece
/// If in check by a slider, push mask is squares between ally king and enemy slider
/// If in check by non slider, push mask is 0
void MoveGen::set_push_mask(){
    uint ally_king_sq = get_lsb(ally_king);
    auto checker_sq = get_lsb(checkers);
    auto checker = board->get_piece_on_square(checker_sq);

    if(is_bishop(checker)){               
        push_mask = mask_opposing_rays(checker_sq, ally_king_sq, 4, 7);
    } else if(is_queen(checker)){
        push_mask = mask_opposing_rays(checker_sq, ally_king_sq, 0, 7);
    } else if(is_rook(checker)){
        push_mask = mask_opposing_rays(checker_sq, ally_king_sq, 0, 3);
    } else {
        push_mask = 0;
    }
}

U64 MoveGen::mask_opposing_rays(uint sq1, uint sq2, int start, int end){
    U64 mask = 0;
    dirInfo info;

    while((mask == 0) && (start <= end)){
        info = dir_info[start];
        mask = RAYS[info.dir][sq1] & RAYS[info.opp_dir][sq2];
        start ++;
    }

    return mask;
}

bool MoveGen::slider_piece(const piece_names& piece){
    return is_bishop(piece) | is_queen(piece) | is_rook(piece);
}

bool MoveGen::is_bishop(const piece_names& piece){
    return (piece == B) | (piece == b);
}

bool MoveGen::is_rook(const piece_names& piece){
    return (piece == R) | (piece == r);
}

bool MoveGen::is_queen(const piece_names& piece){
    return (piece == Q) | (piece == q);
}

/// Given a king attack set, look through it and return a bitboard of those squares in the attack set that are attacked by enemy piece
void MoveGen::set_king_danger_squares(U64 attack_set, int king_colour){
    king_danger_squares = 0;
    uint lsb;

    while(attack_set){
        lsb = get_lsb(attack_set);

        if(get_attackers(lsb, ~king_colour)){
            king_danger_squares |= set_bit(lsb);
        }

        attack_set &= attack_set-1;
    }
}

/// Given a square, and a piece colour, return a bitboard of all pieces of that colour attacking that square
U64 MoveGen::get_attackers(uint square, const int colour){
    U64 out = 0;

    if(colour){
        out |= (knight_attack_set[square] & black_knights);
        out |= (king_attack_set[square] & black_king);
        out |= set_bit(square+7) & ~A_FILE & black_pawns;
        out |= set_bit(square+9) & ~H_FILE & black_pawns;
        out |= get_queen_attacks(whites_minus_king | blacks, square) & black_queens;
        out |= get_rook_attacks(whites_minus_king | blacks, square) & black_rooks;
        out |= get_bishop_attacks(whites_minus_king | blacks, square) & black_bishops;                
    } else {
        out |= (knight_attack_set[square] & white_knights);
        out |= (king_attack_set[square] & white_king);
        out |= set_bit(square-9) & ~A_FILE & white_pawns;
        out |= set_bit(square-7) & ~H_FILE & white_pawns;
        out |= get_queen_attacks(blacks_minus_king | whites, square) & white_queens;
        out |= get_rook_attacks(blacks_minus_king | whites, square) & white_rooks;
        out |= get_bishop_attacks(blacks_minus_king | whites, square) & white_bishops;    
    }

    return (out);
}

/// Produce bitboard of all pieces giving ally king check, and return the number of checkers
uint MoveGen::get_checkers(){
    uint ally_king_sq = get_lsb(ally_king);
    checkers = get_attackers(ally_king_sq, ~turn);

    return count_set_bits(checkers);
}

void MoveGen::P_moves(){
    white_pawns &= ~pinned_pieces;

    // forward 1
    tos = (white_pawns << 8) & ~RANK(8) & ~occupied & push_mask;   
    create_pawn_moves(tos,-8,0);

    // forward 2           
    tos = ((white_pawns & RANK(2) & ~((occupied & RANK(3)) >> 8)) << 16) & ~occupied & push_mask;
    create_pawn_moves(tos,-16,1);

    // right captures
    tos = (white_pawns << 7) & ~RANK(8) & ~A_FILE & blacks_minus_king & capture_mask;
    create_pawn_moves(tos,-7,4); 

    // left captures
    tos = (white_pawns << 9) & ~RANK(8) & ~H_FILE & blacks_minus_king & capture_mask;
    create_pawn_moves(tos,-9,4); 

    // promotion forward 1
    tos = (white_pawns << 8) & RANK(8) & ~occupied & push_mask;
    for(int i = 0; i < 4; ++i){
        create_pawn_moves(tos,-8,p_flags[i]);
    }

    // promotion right captures
    tos = (white_pawns << 7) & RANK(8) & ~A_FILE & blacks_minus_king & capture_mask;
    for(int i = 0; i < 4; ++i){
        create_pawn_moves(tos,-7,pc_flags[i]);
    }

    // promotion left captures
    tos = (white_pawns << 9) & RANK(8) & ~H_FILE & blacks_minus_king & capture_mask;
    for(int i = 0; i < 4; ++i){
        create_pawn_moves(tos,-9,pc_flags[i]);
    }

    // enpassant captures
    if((board->get_prev_move(prev_move) == 0) && (prev_move.get_flags() == 1)){
        pawn_bitboard = set_bit(prev_move.get_to());

        if(!(pawn_bitboard & whites)){
            // black pawn made the double pawn push, maybe we can capture it via enpassant
            pawn_bitboard &= capture_mask | (push_mask >> 8);

            if(!ep_discovered_check(pawn_bitboard | (pawn_bitboard >> 1), black_rooks, black_queens)){
                // capture by white pawn to the right
                tos = (white_pawns & ((pawn_bitboard & ~H_FILE) >> 1)) << 9;
                create_pawn_moves(tos,-9,5);
            }

            if(!ep_discovered_check(pawn_bitboard | (pawn_bitboard << 1), black_rooks, black_queens)){
                // capture by white pawn to the left
                tos = (white_pawns & ((pawn_bitboard & ~A_FILE) << 1)) << 7;
                create_pawn_moves(tos,-7,5);
            }

        }
    }      
}

void MoveGen::p_moves(){
    black_pawns &= ~pinned_pieces;
    // forward 1
    tos = (black_pawns >> 8) & ~occupied & ~RANK(1) & push_mask;  
    create_pawn_moves(tos,8,0);
    
    // forward 2
    tos = ((black_pawns & RANK(7) & ~((occupied & RANK(6)) << 8)) >> 16) & ~occupied & push_mask;
    create_pawn_moves(tos,16,1);

    // right captures
    tos = (black_pawns >> 9) & ~RANK(1) & ~A_FILE & whites_minus_king & capture_mask;
    create_pawn_moves(tos,9,4); 

    // left captures
    tos = (black_pawns >> 7) & ~RANK(1) & ~H_FILE & whites_minus_king & capture_mask;
    create_pawn_moves(tos,7,4); 

    // promotion forward 1
    tos = (black_pawns >> 8) & RANK(1) & ~occupied & push_mask;
    for(int i = 0; i < 4; ++i){
        create_pawn_moves(tos,8,p_flags[i]);
    }

    // promotion right captures
    tos = (black_pawns >> 9) & RANK(1) & ~A_FILE & whites_minus_king & capture_mask;
    for(int i = 0; i < 4; ++i){
        create_pawn_moves(tos,9,pc_flags[i]); 
    }
    
    // promotion left captures
    tos = (black_pawns >> 7) & RANK(1) & ~H_FILE & whites_minus_king & capture_mask;
    for(int i = 0; i < 4; ++i){
        create_pawn_moves(tos,7,pc_flags[i]); 
    }

    // enpassant captures
    if((board->get_prev_move(prev_move) == 0) && (prev_move.get_flags() == 1)){
        pawn_bitboard = set_bit(prev_move.get_to());

        if(!(pawn_bitboard & blacks)){
            // white pawn made the double pawn push, maybe we can capture it via enpassant
            pawn_bitboard &= (capture_mask | (push_mask << 8));

            if(!ep_discovered_check(pawn_bitboard | (pawn_bitboard >> 1), white_rooks, white_queens)){
                // capture by black pawn to the right
                tos = (black_pawns & ((pawn_bitboard & ~H_FILE) >> 1)) >> 7;
                create_pawn_moves(tos,7,5);
            }

            if(!ep_discovered_check(pawn_bitboard | (pawn_bitboard << 1), white_rooks, white_queens)){
                // capture by black pawn to the left
                tos = (black_pawns & ((pawn_bitboard & ~A_FILE) << 1)) >> 9;
                create_pawn_moves(tos,9,5);
            }
        }

    }
}

bool MoveGen::ep_discovered_check(U64 ep_mask, U64 enemy_rooks, U64 enemy_queens){
    auto ally_king_sq = get_lsb(ally_king);
    auto occupancy = occupied & ~ep_mask;

    auto horizontal_mask = get_rank_attacks(occupancy, ally_king_sq);

    return (((get_rook_attacks(occupancy, ally_king_sq) & enemy_rooks) | (get_queen_attacks(occupancy, ally_king_sq) & enemy_queens)) & horizontal_mask) != 0;
}

void MoveGen::N_moves(){
    auto knights = white_knights & ~pinned_pieces;
    U64 attack_set;
    uint from;

    while(knights){
        from =  get_lsb(knights);
        attack_set = knight_attack_set[from];

        // tos for knight capture
        tos = attack_set & blacks_minus_king & capture_mask;
        create_other_moves(tos, from, 4);

        // tos for quiet knight move
        tos = attack_set & ~occupied & push_mask;
        create_other_moves(tos, from, 0);

        knights &= knights - 1;
    }
}

void MoveGen::n_moves(){
    auto knights = black_knights & ~pinned_pieces;
    U64 attack_set;
    uint from;

    while(knights){
        from =  get_lsb(knights);
        attack_set = knight_attack_set[from];

        // tos for knight capture
        tos = attack_set & whites_minus_king & capture_mask;
        create_other_moves(tos, from, 4);

        // tos for quiet knight move
        tos = attack_set & ~occupied & push_mask;
        create_other_moves(tos, from, 0);

        knights &= knights - 1;
    }
}

void MoveGen::K_moves(){
    checkers_count = get_checkers();

    U64 attack_set, can_capture, can_push;
    uint from;
    king_danger_squares = 0;

    from = get_lsb(ally_king);
    attack_set = king_attack_set[from];

    // filter out king danger squares
    set_king_danger_squares(attack_set, turn);

    can_capture = blacks_minus_king & ~king_danger_squares;
    can_push = ~occupied & ~king_danger_squares;

    // tos for king capture
    tos = (attack_set & can_capture);
    create_other_moves(tos, from, 4);

    // tos for quiet king move
    tos = (attack_set & can_push);
    create_other_moves(tos, from, 0);

    if(checkers_count == 0){
        if(board->has_castling_rights(K_castle)){
            if(!get_bit(occupied,2) && !get_bit(occupied,1) && !get_attackers(2,BLACK) && !get_attackers(1,BLACK)){
                create_other_moves(set_bit(1), 3, 2);
            }
        }

        if(board->has_castling_rights(Q_castle)){
            if(!get_bit(occupied,4) && !get_bit(occupied,5) && !get_bit(occupied,6) && !get_attackers(4,BLACK) && !get_attackers(5,BLACK)){
                create_other_moves(set_bit(5), 3, 3);
            }
        }
    }
}

void MoveGen::k_moves(){
    checkers_count = get_checkers();

    U64 attack_set, can_capture, can_push;
    uint from;
    king_danger_squares = 0;

    from = get_lsb(ally_king);
    attack_set = king_attack_set[from];

    // filter out king danger squares
    set_king_danger_squares(attack_set, turn);

    can_capture = whites_minus_king & ~king_danger_squares;
    can_push = ~occupied & ~king_danger_squares;

    // tos for king capture
    tos = (attack_set & can_capture);
    create_other_moves(tos, from, 4);

    // tos for quiet king move
    tos = (attack_set & can_push);
    create_other_moves(tos, from, 0);

    if(checkers_count == 0){
        if(board->has_castling_rights(k_castle)){  
            if(!get_bit(occupied,58) && !get_bit(occupied,57) && !get_attackers(58,WHITE) && !get_attackers(57,WHITE)){
                create_other_moves(set_bit(57), 59, 2);
            }
        }

        if(board->has_castling_rights(q_castle)){
            if(!get_bit(occupied,60) && !get_bit(occupied,61) && !get_bit(occupied,62) && !get_attackers(60,WHITE) && !get_attackers(61,WHITE)){
                create_other_moves(set_bit(61), 59, 3);
            }
        }
    }
}

void MoveGen::R_moves(){
    auto rooks = white_rooks & ~pinned_pieces;
    uint from;
    U64 attack_set;

    while(rooks){
        from = get_lsb(rooks);

        attack_set = get_rook_attacks(occupied, from) & ~whites;

        // rook captures
        tos = attack_set & blacks_minus_king & capture_mask;

        create_other_moves(tos, from, 4);

        // rook quiet moves
        tos = attack_set & ~occupied & push_mask;
        create_other_moves(tos, from, 0);

        rooks &= rooks - 1;
    }
}

void MoveGen::r_moves(){
    auto rooks = black_rooks & ~pinned_pieces;
    uint from;
    U64 attack_set;

    while(rooks){
        from = get_lsb(rooks);

        attack_set = get_rook_attacks(occupied, from) & ~blacks;

        // rook captures
        tos = attack_set & whites_minus_king & capture_mask;

        create_other_moves(tos, from, 4);

        // rook quiet moves
        tos = attack_set & ~occupied & push_mask;
        create_other_moves(tos, from, 0);

        rooks &= rooks - 1;
    }
}

void MoveGen::B_moves(){
    auto bishops = white_bishops & ~pinned_pieces;
    uint from;
    U64 attack_set;

    while(bishops){
        from = get_lsb(bishops);

        attack_set = get_bishop_attacks(occupied, from) & ~whites;

        // bishop captures
        tos = attack_set & blacks_minus_king & capture_mask;

        create_other_moves(tos, from, 4);

        // rook quiet moves
        tos = attack_set & ~occupied & push_mask;
        create_other_moves(tos, from, 0);

        bishops &= bishops - 1;
    }
}

void MoveGen::b_moves(){
    auto bishops = black_bishops & ~pinned_pieces;
    uint from;
    U64 attack_set;

    while(bishops){
        from = get_lsb(bishops);

        attack_set = get_bishop_attacks(occupied, from) & ~blacks;

        // bishop captures
        tos = attack_set & whites_minus_king & capture_mask;

        create_other_moves(tos, from, 4);

        // rook quiet moves
        tos = attack_set & ~occupied & push_mask;
        create_other_moves(tos, from, 0);

        bishops &= bishops - 1;
    }
}

void MoveGen::Q_moves(){
    auto queens = white_queens & ~pinned_pieces;
    uint from;
    U64 attack_set;

    while(queens){
        from = get_lsb(queens);

        attack_set = get_queen_attacks(occupied, from) & ~whites;

        // queen captures
        tos = attack_set & blacks_minus_king & capture_mask;

        create_other_moves(tos, from, 4);

        // queen quiet moves
        tos = attack_set & ~occupied & push_mask;
        create_other_moves(tos, from, 0);

        queens &= queens - 1;
    }

}

void MoveGen::q_moves(){
    auto queens = black_queens & ~pinned_pieces;
    uint from;
    U64 attack_set;

    while(queens){
        from = get_lsb(queens);

        attack_set = get_queen_attacks(occupied, from) & ~blacks;

        // queen captures
        tos = attack_set & whites_minus_king & capture_mask;


        create_other_moves(tos, from, 4);

        // queen quiet moves
        tos = attack_set & ~occupied & push_mask;
        create_other_moves(tos, from, 0);

        queens &= queens - 1;
    }
}

/// Given a bitboard of destination squares, a pointer to the board state, an offset of calculate from square, and a flag to 
/// indicate move type, add that move to the list of valid moves in board state
void MoveGen::create_pawn_moves(U64 tos, int offset, uint flag){
    uint to;
    while(tos){
        to =  get_lsb(tos);

        board->add_valid_move(Move(to+offset, to, flag));

        tos &= tos-1;
    }
}

/// This receives the actual from square, as its the same for all tos that are passed to it
void MoveGen::create_other_moves(U64 tos, uint from, uint flag){
    uint to;

    while(tos){
        to =  get_lsb(tos);

        board->add_valid_move(Move(from, to, flag));

        tos &= tos-1;
    }
}

U64 MoveGen::get_positive_ray_attacks(U64 occupied, dirs dir, uint square){
    auto attacks = RAYS[dir][square];
    auto blockers = occupied & attacks;

    square = get_lsb(blockers | 0x8000000000000000);
    attacks ^= RAYS[dir][square];

    return attacks;
}

U64 MoveGen::get_negative_ray_attacks(U64 occupied, dirs dir, uint square){
    auto attacks = RAYS[dir][square];
    auto blockers = occupied & attacks;

    square = get_msb(blockers | 1ULL);
    attacks ^= RAYS[dir][square];

    return attacks;
}
