#include "movegen.h"

/*Use square directly as index, since table is the same even when rotated by 180 degrees
    Need to consider this since table indexing for me is set out like so:
    63 62 61 60 59 58 57 56
    55 54 53 52 51 50 49 48
    ......
    7  6  5  4  3  2  1  0
*/
int RBits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

/*Use square directly as index, since table is the same even when rotated by 180 degrees
    Need to consider this since table indexing for me is set out like so:
    63 62 61 60 59 58 57 56
    55 54 53 52 51 50 49 48
    ......
    7  6  5  4  3  2  1  0
*/
int BBits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};

U64 rook_moves[64][4096] = {0ULL};
U64 bishop_moves[64][4096] = {0ULL}; 

U64 bishop_magics[64] = {4728788573581738048, 5733416633603686400, 5229823810539634944, 6059630168275289221, 3202358682279544867, 
    5666936944433561728, 11761487987115557892, 2348949397041258504, 11614290900869776640, 8571211975631315008, 
    4979054955174428676, 5959959978772529282, 123437781283504128, 7141400599083354112, 1812159642724556800, 
    1006995981577769472, 4990313312423772416, 5822270525863494145, 6960315217568088080, 6072617855688507520, 
    4974790954606198792, 3577828800497139714, 15597091608452401154, 4370327149765726464, 2312167826680449025, 
    13912819988931627040, 5785267501407437312, 2507387889824956480, 103301609735340040, 459651936544620672, 
    163402856292552704, 2531974106828570884, 6570886340620652545, 4217636689277753348, 891150919884081152, 
    1489569992014299664, 5799512345640960256, 1478634511329002242, 7191154685453813766, 9306723057480959236, 
    1806234169480265732, 4380878496618251521, 17461018851316533248, 11233509228138203136, 8715133996981354624, 
    4854348990617157888, 1389696007914541570, 4688417391593588994, 14068137530891502080, 10688468016994062336, 
    409071523086737408, 1032812357711302824, 2566052056927110661, 999837651843022880, 5703923311846885892, 
    639519947995165696, 7211387954460572164, 3024187967505178628, 5955734126153237536, 4474412103514064898, 
    984941316833551873, 245868809644737026, 5062156173791265792, 9114183617453556480};

U64 rook_magics[64] = {468375883830476800, 2756212595361976320, 2341881839325544832, 360314996002979872, 5044066853698162696, 
    2449970515533103616, 4755804255931664388, 11493191915369660800, 477803773512794114, 1451848205322485888, 
    455426654357980673, 12646670780923381792, 6921469910688333840, 7274720876336251136, 8081991255401988352, 
    1764848122531578628, 1214126369148321796, 1445939154931286080, 5453578773561663488, 36382840181620768, 
    6739638491769996289, 1062006186783671296, 3514852801113034882, 908928879668830340, 1661548995138453568, 
    13632502771927220864, 4065133086207188993, 6127711282374689312, 1349795714465005824, 11007079561272230912, 
    2759678992069953538, 4675873866579644500, 6827494143636275328, 3283511719271350272, 4682409905213153280, 
    5769392771097956352, 3198018631187306496, 14003380641689764864, 7408529283515158568, 2270881580656365713, 
    514888105988554752, 1529042579944259588, 5809679536962797584, 8288287571251953680, 8577951857522049040, 
    2885963365714165784, 5288360418451128336, 5631201592249286657, 8541196265373827456, 639883336168317440, 
    8516609536478545408, 5655116211135971584, 15302106484435878016, 10633896196665509889, 6686447200015221760, 
    6420254574959706624, 2272436422877347841, 6712052571028258946, 429304590492106882, 42103599306637321, 
    1198520589101565954, 2363545965200408577, 470736666308397068, 4984485903327297794};

U64 movegen_helpers::get_bishop_occupancies(int square){
    U64 attacks = 0ULL;
    int rank = square % 8;
    int file = square / 8;
    int r, f;

    for(r = rank-1, f = file-1; (r >= 1) && (f >= 1); r--, f--) attacks |= set_bit(8*f+r);
    for(r = rank+1, f = file+1; (r <= 6) && (f <= 6); r++, f++) attacks |= set_bit(8*f+r);
    for(r = rank+1, f = file-1; (r <= 6) && (f >= 1); r++, f--) attacks |= set_bit(8*f+r);
    for(r = rank-1, f = file+1; (r >= 1) && (f <= 6); r--, f++) attacks |= set_bit(8*f+r);

    return attacks;
}

U64 movegen_helpers::get_rook_occupancies(int square){
    U64 attacks = 0ULL;
    int rank = square % 8;
    int file = square / 8;
    int r, f;

    for(r = rank-1; r >= 1; r--) attacks |= set_bit(8*file+r);
    for(r = rank+1; r <= 6; r++) attacks |= set_bit(8*file+r);
    for(f = file-1; f >= 1; f--) attacks |= set_bit(8*f+rank);
    for(f = file+1; f <= 6; f++) attacks |= set_bit(8*f+rank);

    return attacks;
}

U64 movegen_helpers::get_bishop_attacks(int square, U64 blockers){
    U64 sq_bb, attacks = 0ULL;
    int rank = square % 8;
    int file = square / 8;
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

U64 movegen_helpers::get_rook_attacks(int square, U64 blockers){
    U64 sq_bb, attacks = 0ULL;
    int rank = square % 8;
    int file = square / 8;
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

U64 movegen_helpers::get_blocker_config(int index, int bits_in_attack_mask, U64 attack_mask){
    U64 occupancy = 0ULL;
    int square;

    for(int i = 0; i<bits_in_attack_mask;++i){
        square = get_lsb(attack_mask);
        attack_mask &= (attack_mask-1);

        if(index & (1 << i)){occupancy |= set_bit(square);}
    }

    return occupancy;
}

int movegen_helpers::transform_to_key(U64 b, U64 magic, int index_bits) {
    return (int)((b * magic) >> (64 - index_bits));
}

U64 movegen_helpers::find_magic(int sq, int index_bits, bool for_bishop){
    // these are set to 4096 because these are the highest number of blocker configurations that are possible on the board
    U64 occupancy, attack_masks[4096], blocker_masks[4096], used[4096], magic;

    unsigned int i = 0, k = 0, j = 0;
    bool fail = false;

    occupancy = for_bishop ? get_bishop_occupancies(sq) : get_rook_occupancies(sq);

    for(; i < set_bit(index_bits); ++i){
        blocker_masks[i] = get_blocker_config(i, index_bits, occupancy);
        attack_masks[i] = for_bishop ? get_bishop_attacks(sq, blocker_masks[i]) : get_rook_attacks(sq, blocker_masks[i]);
    }

    for(; k < 100000000; ++k){
        magic = RAND64 & RAND64 & RAND64;  // this is done to reduce the number of bits in the random number generated
        // I think this is here because the lowest number of bits in the occupancy set is 5, and if this condition is passed, 
        // then the amount of useful bits in the key will not be enough to represent that blocker
        if(count_set_bits((occupancy * magic) & 0xFF00000000000000ULL) < 6) continue;
        for(i = 0; i < 4096; ++i) used[i] = 0ULL;
        
        fail = false;

        // attempt to fill up all attack sets without any destructive collisions 
        for(i = 0; !fail && (i < set_bit(index_bits)); ++i){
            j = movegen_helpers::transform_to_key(blocker_masks[i], magic, index_bits);

            if(used[j] == 0ULL) used[j] = attack_masks[i]; // fill in attack set
            else if (used[j] != attack_masks[i]) fail = true;  // destructive collision, fail 
        }

        if(!fail){
            if(debug) std::cout << "found magic for " << sq << std::endl;
            return magic;  // this magic number worked!
        }
    }

    if(debug) std::cout << "no magic found for " << sq << std::endl;
    return 0ULL; // couldn't find magic number that works

}

/// Given a board state, generate all valid moves in that state
MoveGen::MoveGen() : prev_move(0,0,0) {
}

std::vector<Move> MoveGen::generate_moves(bool _captures_only){
    // initialisations
    white_pawns = board->bitboards[P];
    black_pawns = board->bitboards[p];
    white_king = board->bitboards[K];
    black_king = board->bitboards[k];
    white_knights = board->bitboards[N];
    black_knights = board->bitboards[n];

    white_bishops = board->bitboards[B];
    white_queens = board->bitboards[Q];
    white_rooks = board->bitboards[R];

    black_bishops = board->bitboards[b];
    black_queens = board->bitboards[q];
    black_rooks = board->bitboards[r];

    whites = white_pawns | white_king | white_knights | white_bishops | white_queens | white_rooks;
    blacks = black_pawns | black_king | black_knights | black_bishops | black_queens | black_rooks;

    pawns = white_pawns | black_pawns;

    occupied = whites | blacks;
    blacks_minus_king = blacks & ~black_king;
    whites_minus_king = whites & ~white_king;

    turn = board->get_turn();
    legal_moves.clear();
    pinned_pieces = 0;

    if(_captures_only){
        captures_only = _captures_only;
        generate_legal_captures();   
    } else {
        generate_legal_moves();
    }

    return legal_moves;
}

bool MoveGen::move_is_legal(Move& move){
    for(Move v_move : legal_moves){
        if(v_move == move){return true;}
    }
    
    return false;
}

std::vector<Move> MoveGen::get_legal_moves(){
    return legal_moves;
}

bool MoveGen::no_legal_moves(){
    return (legal_moves.size() == 0) || (legal_moves[0] == Move(0,0,0)); 
}

/// generate only legal capture moves
/// called during quiescence search
void MoveGen::generate_legal_captures(){

    if(turn){
        ally_king = white_king;
        enemy_king = black_king;
        ally_pieces = whites_minus_king;
        diag_pinners = black_bishops | black_queens;
        nondiag_pinners = black_rooks | black_queens;

        checkers_count = get_checkers();

        K_captures_moves();
    } else {
        ally_king = black_king;
        enemy_king = white_king;
        ally_pieces = blacks_minus_king;    
        diag_pinners = white_bishops | white_queens;
        nondiag_pinners = white_rooks | white_queens;

        checkers_count = get_checkers();

        k_captures_moves();
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

        if(turn){
            N_captures_moves();
            R_captures_moves();
            B_captures_moves();
            Q_captures_moves();
            P_captures_moves();

        } else {
            n_captures_moves();
            r_captures_moves();
            b_captures_moves();
            q_captures_moves();
            p_captures_moves();  
        }
    }

    captures_only = false;
}

void MoveGen::generate_legal_moves(){

    if(turn){
        ally_king = white_king;
        enemy_king = black_king;
        ally_pieces = whites_minus_king;
        diag_pinners = black_bishops | black_queens;
        nondiag_pinners = black_rooks | black_queens;

        checkers_count = get_checkers();

        K_captures_moves();
        K_quiet_moves();
    } else {
        ally_king = black_king;
        enemy_king = white_king;
        ally_pieces = blacks_minus_king;
        diag_pinners = white_bishops | white_queens;
        nondiag_pinners = white_rooks | white_queens;

        checkers_count = get_checkers();

        k_captures_moves();
        k_quiet_moves();
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

        if(turn){
            N_captures_moves();
            R_captures_moves();
            B_captures_moves();
            Q_captures_moves();
            P_captures_moves();

            N_quiet_moves();
            R_quiet_moves();
            B_quiet_moves();
            Q_quiet_moves();
            P_quiet_moves();

        } else {
            n_captures_moves();
            r_captures_moves();
            b_captures_moves();
            q_captures_moves();
            p_captures_moves();

            n_quiet_moves();
            r_quiet_moves();
            b_quiet_moves();
            q_quiet_moves();
            p_quiet_moves(); 
        }
    }
}

/// Setup a bitboard of all pinned pieces on the board. This mask is used to remove pinned pieces when such that moves aren't generated for them
/// in main move generator. Instead, pinned pieces' moves are generated separately at right after king moves are generated
void MoveGen::get_pinned_pieces(){ 
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
    uint pinned_sq = get_lsb(pinned_bitboard);
    piece_names pinned_piece_name = board->get_piece_on_square(pinned_sq);

    if(valid_slider_pin(pinned_piece_name, _ray_type)){
        create_other_moves(set_bit(pinner_sq), pinned_sq, 4);
        if(!captures_only){
            create_other_moves(possible_pin & ~pinned_bitboard, pinned_sq, 0);
        }
    } else if (pinned_piece_name == P){
        // right / left captures
        tos = (pinned_bitboard << 7) & set_bit(pinner_sq);
        create_pawn_moves(tos,-7,4);

        tos = (pinned_bitboard << 9) & set_bit(pinner_sq);
        create_pawn_moves(tos,-9,4);

        if(!captures_only){
            // quiet moves
            tos = (pinned_bitboard << 8) & possible_pin;
            create_pawn_moves(tos, -8, 0);
            
            tos = ((pinned_bitboard & RANK(2)) << 16) & possible_pin;
            create_pawn_moves(tos, -16, 1);
        }   

    } else if(pinned_piece_name == p){
        // right / left captures
        tos = (pinned_bitboard >> 9) & set_bit(pinner_sq);
        create_pawn_moves(tos,9,4);

        tos = (pinned_bitboard >> 7) & set_bit(pinner_sq);
        create_pawn_moves(tos,7,4);

        if(!captures_only){
            // quiet moves
            tos = (pinned_bitboard >> 8) & possible_pin;
            create_pawn_moves(tos, 8, 0);
            
            tos = ((pinned_bitboard & RANK(7)) >> 16) & possible_pin;
            create_pawn_moves(tos, 16, 1);
        }
    }
}

/// only call if in check by one piece
/// If in check by a slider, push mask is squares between ally king and enemy slider
/// If in check by non slider, push mask is 0
void MoveGen::set_push_mask(){
    uint ally_king_sq = get_lsb(ally_king);
    uint checker_sq = get_lsb(checkers);

    piece_names checker = board->get_piece_on_square(checker_sq);

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
void MoveGen::set_king_danger_squares(U64 attack_set, colour king_colour){
    king_danger_squares = 0;
    squares lsb;

    while(attack_set){
        lsb = (squares)get_lsb(attack_set);

        if(get_attackers(lsb, (colour)(1 - king_colour))){
            king_danger_squares |= set_bit(lsb);
        }

        attack_set &= attack_set-1;
    }
}

/// Given a square, and a piece colour, return a bitboard of all pieces of that colour attacking that square
U64 MoveGen::get_attackers(squares square, colour colour){
    U64 out = 0ULL, bblockers, rblockers, rookmoves, bishopmoves, blocker_mask, king, knights, rooks, bishops, queens;
    int rkey, bkey;

    if(colour == black){
        blocker_mask = whites_minus_king | blacks;
        king = black_king;
        knights = black_knights;
        rooks = black_rooks;
        bishops = black_bishops;
        queens = black_queens;

        out |= set_bit(square + 7) & ~A_FILE & black_pawns;
        out |= set_bit(square + 9) & ~H_FILE & black_pawns;

    } else {
        blocker_mask = blacks_minus_king | whites;
        king = white_king;
        knights = white_knights;
        rooks = white_rooks;
        bishops = white_bishops;
        queens = white_queens;

        out |= set_bit(square - 9) & ~A_FILE & white_pawns;
        out |= set_bit(square - 7) & ~H_FILE & white_pawns;
    }

    rblockers = movegen_helpers::get_rook_occupancies(square) & blocker_mask;
    bblockers = movegen_helpers::get_bishop_occupancies(square) & blocker_mask;

    rkey = movegen_helpers::transform_to_key(rblockers, rook_magics[square], RBits[square]);
    bkey = movegen_helpers::transform_to_key(bblockers, bishop_magics[square], BBits[square]);

    rookmoves = rook_moves[square][rkey];
    bishopmoves = bishop_moves[square][bkey];

    out |= rookmoves & rooks;
    out |= bishopmoves & bishops;
    out |= (rookmoves | bishopmoves) & queens;
    out |= (knight_attack_set[square] & knights);
    out |= (king_attack_set[square] & king);

    return (out);
}

/// Given a square, and a piece colour, return a bitboard of all pawns of that colour attacking that square
U64 MoveGen::get_pawn_attackers(uint square, colour colour){
    U64 pawn_attackers = 0;

    if(colour == black){
        pawn_attackers |= set_bit(square+7) & ~A_FILE & black_pawns;
        pawn_attackers |= set_bit(square+9) & ~H_FILE & black_pawns;             
    } else {
        pawn_attackers |= set_bit(square-9) & ~A_FILE & white_pawns;
        pawn_attackers |= set_bit(square-7) & ~H_FILE & white_pawns;   
    }

    return pawn_attackers;
}

/// Produce bitboard of all pieces giving ally king check, and return the number of checkers
uint MoveGen::get_checkers(){
    squares ally_king_sq = (squares)get_lsb(ally_king);
    checkers = get_attackers(ally_king_sq, (colour)(1 - turn));

    return count_set_bits(checkers);
}

bool MoveGen::ep_discovered_check(U64 ep_mask, U64 enemy_rooks, U64 enemy_queens){
    int ally_king_sq = get_lsb(ally_king), key;
    U64 occupied_minus_ep = occupied & ~ep_mask;
    U64 horizontal_mask = get_rank_attacks(occupied_minus_ep, ally_king_sq);
    return (((get_rook_attacks(occupied_minus_ep, ally_king_sq) & enemy_rooks) | (get_queen_attacks(occupied_minus_ep, ally_king_sq) & enemy_queens)) & horizontal_mask) != 0;
}

void MoveGen::P_quiet_moves(){
    white_pawns &= ~pinned_pieces;

    // forward 1
    tos = (white_pawns << 8) & ~RANK(8) & ~occupied & push_mask;   
    create_pawn_moves(tos,-8,0);

    // forward 2           
    tos = ((white_pawns & RANK(2) & ~((occupied & RANK(3)) >> 8)) << 16) & ~occupied & push_mask;
    create_pawn_moves(tos,-16,1);

    // promotion forward 1
    tos = (white_pawns << 8) & RANK(8) & ~occupied & push_mask;

    for(int i = 0; i < 4; ++i){
        create_pawn_moves(tos,-8,p_flags[i]);
    }  
}

void MoveGen::P_captures_moves(){
    white_pawns &= ~pinned_pieces;

    // right captures
    tos = (white_pawns << 7) & ~RANK(8) & ~A_FILE & blacks_minus_king & capture_mask;
    create_pawn_moves(tos,-7,4); 

    // left captures
    tos = (white_pawns << 9) & ~RANK(8) & ~H_FILE & blacks_minus_king & capture_mask;
    create_pawn_moves(tos,-9,4); 

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

void MoveGen::p_quiet_moves(){
    black_pawns &= ~pinned_pieces;

    // forward 1
    tos = (black_pawns >> 8) & ~occupied & ~RANK(1) & push_mask;  
    create_pawn_moves(tos,8,0);
    
    // forward 2
    tos = ((black_pawns & RANK(7) & ~((occupied & RANK(6)) << 8)) >> 16) & ~occupied & push_mask;
    create_pawn_moves(tos,16,1);

    // promotion forward 1
    tos = (black_pawns >> 8) & RANK(1) & ~occupied & push_mask;
    for(int i = 0; i < 4; ++i){
        create_pawn_moves(tos,8,p_flags[i]);
    }

}

void MoveGen::p_captures_moves(){
    black_pawns &= ~pinned_pieces;

    // right captures
    tos = (black_pawns >> 9) & ~RANK(1) & ~A_FILE & whites_minus_king & capture_mask;
    create_pawn_moves(tos,9,4); 

    // left captures
    tos = (black_pawns >> 7) & ~RANK(1) & ~H_FILE & whites_minus_king & capture_mask;
    create_pawn_moves(tos,7,4); 

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

void MoveGen::N_quiet_moves(){
    auto knights = white_knights & ~pinned_pieces;
    U64 attack_set;
    uint from;

    while(knights){
        from =  get_lsb(knights);
        attack_set = knight_attack_set[from];

        // tos for quiet knight move
        tos = attack_set & ~occupied & push_mask;
        create_other_moves(tos, from, 0);

        knights &= knights - 1;
    }
}

void MoveGen::N_captures_moves(){
    auto knights = white_knights & ~pinned_pieces;
    U64 attack_set;
    uint from;

    while(knights){
        from = get_lsb(knights);
        attack_set = knight_attack_set[from];

        // tos for knight capture
        tos = attack_set & blacks_minus_king & capture_mask;
        create_other_moves(tos, from, 4);

        knights &= knights - 1;
    }
}

void MoveGen::n_quiet_moves(){
    auto knights = black_knights & ~pinned_pieces;
    U64 attack_set;
    uint from;

    while(knights){
        from = get_lsb(knights);
        attack_set = knight_attack_set[from];

        // tos for quiet knight move
        tos = attack_set & ~occupied & push_mask;
        create_other_moves(tos, from, 0);

        knights &= knights - 1;
    }
}

void MoveGen::n_captures_moves(){
    auto knights = black_knights & ~pinned_pieces;
    U64 attack_set;
    uint from;

    while(knights){
        from =  get_lsb(knights);
        attack_set = knight_attack_set[from];

        // tos for knight capture
        tos = attack_set & whites_minus_king & capture_mask;
        create_other_moves(tos, from, 4);

        knights &= knights - 1;
    }
}

void MoveGen::K_quiet_moves(){
    uint from = get_lsb(ally_king);
    U64 can_push = ~occupied & ~king_danger_squares;
    U64 attack_set = king_attack_set[from];

    // tos for quiet king move
    tos = attack_set & can_push;
    create_other_moves(tos, from, 0);

    if(checkers_count == 0){
        if(board->has_castling_rights(K_castle)){
            if(!get_bit(occupied,f1) && !get_bit(occupied,g1) && !get_attackers(f1,black) && !get_attackers(g1,black)){
                create_other_moves(set_bit(g1), e1, 2);
            }
        }

        if(board->has_castling_rights(Q_castle)){
            if(!get_bit(occupied,d1) && !get_bit(occupied,c1) && !get_bit(occupied,b1) && !get_attackers(d1,black) && !get_attackers(c1,black)){
                create_other_moves(set_bit(c1), e1, 3);
            }
        }
    }
}

void MoveGen::K_captures_moves(){
    uint from = get_lsb(ally_king);

    U64 attack_set, can_capture;
    attack_set = king_attack_set[from];

    king_danger_squares = 0;

    // filter out king danger squares
    set_king_danger_squares(attack_set, turn);
    can_capture = blacks_minus_king & ~king_danger_squares;
    
    // tos for king capture
    tos = attack_set & can_capture;
    create_other_moves(tos, from, 4);    
}

void MoveGen::k_quiet_moves(){
    uint from = get_lsb(ally_king);
    U64 can_push = ~occupied & ~king_danger_squares;
    U64 attack_set = king_attack_set[from];

    // tos for quiet king move
    tos = (attack_set & can_push);
    create_other_moves(tos, from, 0);

    if(checkers_count == 0){
        if(board->has_castling_rights(k_castle)){  
            if(!get_bit(occupied,f8) && !get_bit(occupied,g8) && !get_attackers(f8,white) && !get_attackers(g8,white)){
                create_other_moves(set_bit(g8), e8, 2);
            }
        }

        if(board->has_castling_rights(q_castle)){
            if(!get_bit(occupied,d8) && !get_bit(occupied,c8) && !get_bit(occupied,b8) && !get_attackers(d8,white) && !get_attackers(c8,white)){
                create_other_moves(set_bit(c8), e8, 3);
            }
        }
    }
}

void MoveGen::k_captures_moves(){
    uint from = get_lsb(ally_king);

    U64 attack_set, can_capture;
    attack_set = king_attack_set[from];

    king_danger_squares = 0;

    // filter out king danger squares
    set_king_danger_squares(attack_set, turn);
    can_capture = whites_minus_king & ~king_danger_squares;

    // tos for king capture
    tos = attack_set & can_capture;
    create_other_moves(tos, from, 4);  
}

void MoveGen::R_quiet_moves(){
    auto rooks = white_rooks & ~pinned_pieces;
    uint from, key;
    U64 attack_set, blockers;

    while(rooks){
        from = get_lsb(rooks);

        blockers = movegen_helpers::get_rook_occupancies(from) & occupied;
        key = movegen_helpers::transform_to_key(blockers, rook_magics[from], RBits[from]);
        attack_set = rook_moves[from][key];

        // rook quiet moves
        tos = attack_set & ~occupied & push_mask;
        create_other_moves(tos, from, 0);

        rooks &= rooks - 1;
    }
}

void MoveGen::R_captures_moves(){
    auto rooks = white_rooks & ~pinned_pieces;
    uint from, key;
    U64 attack_set, blockers;

    while(rooks){
        from = get_lsb(rooks);

        blockers = movegen_helpers::get_rook_occupancies(from) & occupied;
        key = movegen_helpers::transform_to_key(blockers, rook_magics[from], RBits[from]);
        attack_set = rook_moves[from][key];  

        // rook captures
        tos = attack_set & blacks_minus_king & capture_mask;

        create_other_moves(tos, from, 4);

        rooks &= rooks - 1;
    }
}

void MoveGen::r_quiet_moves(){
    auto rooks = black_rooks & ~pinned_pieces;
    uint from, key;
    U64 attack_set, blockers;

    while(rooks){
        from = get_lsb(rooks);

        blockers = movegen_helpers::get_rook_occupancies(from) & occupied;
        key = movegen_helpers::transform_to_key(blockers, rook_magics[from], RBits[from]);
        attack_set = rook_moves[from][key];  

        // rook quiet moves
        tos = attack_set & ~occupied & push_mask;
        create_other_moves(tos, from, 0);

        rooks &= rooks - 1;
    }
}

void MoveGen::r_captures_moves(){
    auto rooks = black_rooks & ~pinned_pieces;
    uint from, key;
    U64 attack_set, blockers;

    while(rooks){
        from = get_lsb(rooks);

        blockers = movegen_helpers::get_rook_occupancies(from) & occupied;
        key = movegen_helpers::transform_to_key(blockers, rook_magics[from], RBits[from]);
        attack_set = rook_moves[from][key];  

        // rook captures
        tos = attack_set & whites_minus_king & capture_mask;
        create_other_moves(tos, from, 4);

        rooks &= rooks - 1;
    }
}

void MoveGen::B_quiet_moves(){
    auto bishops = white_bishops & ~pinned_pieces;
    uint from, key;
    U64 attack_set, blockers;

    while(bishops){
        from = get_lsb(bishops);

        blockers = movegen_helpers::get_bishop_occupancies(from) & occupied;
        key = movegen_helpers::transform_to_key(blockers, bishop_magics[from], BBits[from]);
        attack_set = bishop_moves[from][key];  

        // bishop quiet moves
        tos = attack_set & ~occupied & push_mask;
        create_other_moves(tos, from, 0);

        bishops &= bishops - 1;
    }
}

void MoveGen::B_captures_moves(){
    auto bishops = white_bishops & ~pinned_pieces;
    uint from, key;
    U64 attack_set, blockers;

    while(bishops){
        from = get_lsb(bishops);

        blockers = movegen_helpers::get_bishop_occupancies(from) & occupied;
        key = movegen_helpers::transform_to_key(blockers, bishop_magics[from], BBits[from]);
        attack_set = bishop_moves[from][key];

        // bishop captures
        tos = attack_set & blacks_minus_king & capture_mask;
        create_other_moves(tos, from, 4);

        bishops &= bishops - 1;
    }
}

void MoveGen::b_quiet_moves(){
    auto bishops = black_bishops & ~pinned_pieces;
    uint from, key;
    U64 attack_set, blockers;

    while(bishops){
        from = get_lsb(bishops);

        blockers = movegen_helpers::get_bishop_occupancies(from) & occupied;
        key = movegen_helpers::transform_to_key(blockers, bishop_magics[from], BBits[from]);
        attack_set = bishop_moves[from][key];

        // bishop quiet moves
        tos = attack_set & ~occupied & push_mask;
        create_other_moves(tos, from, 0);

        bishops &= bishops - 1;
    }
}

void MoveGen::b_captures_moves(){
    auto bishops = black_bishops & ~pinned_pieces;
    uint from, key;
    U64 attack_set, blockers;

    while(bishops){
        from = get_lsb(bishops);

        blockers = movegen_helpers::get_bishop_occupancies(from) & occupied;
        key = movegen_helpers::transform_to_key(blockers, bishop_magics[from], BBits[from]);
        attack_set = bishop_moves[from][key];

        // bishop captures
        tos = attack_set & whites_minus_king & capture_mask;
        create_other_moves(tos, from, 4);

        bishops &= bishops - 1;
    }
}

void MoveGen::Q_quiet_moves(){
    auto queens = white_queens & ~pinned_pieces;
    uint from, rkey, bkey;
    U64 attack_set, rblockers, bblockers;

    while(queens){
        from = get_lsb(queens);

        bblockers = movegen_helpers::get_bishop_occupancies(from) & occupied;
        rblockers = movegen_helpers::get_rook_occupancies(from) & occupied;
        bkey = movegen_helpers::transform_to_key(bblockers, bishop_magics[from], BBits[from]);
        rkey = movegen_helpers::transform_to_key(rblockers, rook_magics[from], RBits[from]);
        attack_set = bishop_moves[from][bkey] | rook_moves[from][rkey];

        // queen quiet moves
        tos = attack_set & ~occupied & push_mask;
        create_other_moves(tos, from, 0);

        queens &= queens - 1;
    }
}

void MoveGen::Q_captures_moves(){
    auto queens = white_queens & ~pinned_pieces;
    uint from, rkey, bkey;
    U64 attack_set, rblockers, bblockers;

    while(queens){
        from = get_lsb(queens);

        bblockers = movegen_helpers::get_bishop_occupancies(from) & occupied;
        rblockers = movegen_helpers::get_rook_occupancies(from) & occupied;
        bkey = movegen_helpers::transform_to_key(bblockers, bishop_magics[from], BBits[from]);
        rkey = movegen_helpers::transform_to_key(rblockers, rook_magics[from], RBits[from]);
        attack_set = bishop_moves[from][bkey] | rook_moves[from][rkey];

        // queen captures
        tos = attack_set & blacks_minus_king & capture_mask;
        create_other_moves(tos, from, 4);

        queens &= queens - 1;
    }
}

void MoveGen::q_quiet_moves(){
    auto queens = black_queens & ~pinned_pieces;
    uint from, rkey, bkey;
    U64 attack_set, rblockers, bblockers;

    while(queens){
        from = get_lsb(queens);

        bblockers = movegen_helpers::get_bishop_occupancies(from) & occupied;
        rblockers = movegen_helpers::get_rook_occupancies(from) & occupied;
        bkey = movegen_helpers::transform_to_key(bblockers, bishop_magics[from], BBits[from]);
        rkey = movegen_helpers::transform_to_key(rblockers, rook_magics[from], RBits[from]);
        attack_set = bishop_moves[from][bkey] | rook_moves[from][rkey];
        
        // queen quiet moves
        tos = attack_set & ~occupied & push_mask;
        create_other_moves(tos, from, 0);

        queens &= queens - 1;
    }
}

void MoveGen::q_captures_moves(){
    auto queens = black_queens & ~pinned_pieces;
    uint from, rkey, bkey;
    U64 attack_set, rblockers, bblockers;

    while(queens){
        from = get_lsb(queens);

        bblockers = movegen_helpers::get_bishop_occupancies(from) & occupied;
        rblockers = movegen_helpers::get_rook_occupancies(from) & occupied;
        bkey = movegen_helpers::transform_to_key(bblockers, bishop_magics[from], BBits[from]);
        rkey = movegen_helpers::transform_to_key(rblockers, rook_magics[from], RBits[from]);
        attack_set = bishop_moves[from][bkey] | rook_moves[from][rkey];

        // queen captures
        tos = attack_set & whites_minus_king & capture_mask;
        create_other_moves(tos, from, 4);

        queens &= queens - 1;
    }
}

/// Given a bitboard of destination squares, a pointer to the board state, an offset of calculate from square, and a flag to 
/// indicate move type, add that move to the list of valid moves in board state
void MoveGen::create_pawn_moves(U64 tos, int offset, uint flag){
    uint to;
    while(tos){
        to =  get_lsb(tos);

        legal_moves.push_back(Move(to+offset, to, flag));

        tos &= tos-1;
    }
}

/// This receives the actual from square, as its the same for all tos that are passed to it
void MoveGen::create_other_moves(U64 tos, uint from, uint flag){
    uint to;

    while(tos){
        to =  get_lsb(tos);

        legal_moves.push_back(Move(from, to, flag));

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
