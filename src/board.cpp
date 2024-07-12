#include <assert.h>
#include "board.h"

Board::Board (){}

/// Make move given as input on the board
void Board::make_move(const Move& move){
    int from = move.get_from();
    int to = move.get_to();
    int flags = move.get_flags();

    piece_names from_piece_name = get_piece_on_square(from);
    piece_names to_piece_name = get_piece_on_square(to);

    colour from_piece_colour = (colour)abs(from_piece_name < 7);

    assert(from_piece_name != None);

    piece_names promo_piece_name;
    piece_names recent_capture = to_piece_name;

    // remove castling rights if king or rook moves
    if(from_piece_name == K){
        castling_rights &= ~(K_castle | Q_castle);
    } else if(from_piece_name == k){
        castling_rights &= ~(k_castle | q_castle);
    } else {
        if((from == 63 && to == 7) || (from == 7 && to == 63)){
            castling_rights &= ~Q_castle;
            castling_rights &= ~q_castle;
        } else if((from == 56 && to == 0) || (from == 0 && to == 56)){
            castling_rights &= ~K_castle;
            castling_rights &= ~k_castle;
        } else if(from == 0 || to == 0){
            castling_rights &= ~K_castle;
        } else if(from == 7 || to == 7){
            castling_rights &= ~Q_castle;
        } else if(from == 56 || to == 56){
            castling_rights &= ~k_castle;
        } else if(from == 63 || to == 63){
            castling_rights &= ~q_castle;
        }
    }

    ep_square = 0;

    if(!move.is_promo()){
        // place piece at to square if not promotion move
        bitboards[from_piece_name] |= set_bit(to);
        add_to_eval(from_piece_name, to);
        
        if(flags == 4){
            capture_piece(to, to_piece_name);
        } else if(flags == 5){
            recent_capture = ep_capture(from_piece_colour, to);
        } else if(flags == 2){
            castle_kingside(from, from_piece_colour);
        } else if(flags == 3){
            castle_queenside(from, from_piece_colour);
        } else if (flags == 1){
            assert(from_piece_name == P || from_piece_name == p);

            hm_clock = 0;
            ep_square = (from_piece_name == P) ? to - 8 : to + 8;

        } else {
            // other quiet moves
            hm_clock++;
        }   
        
    } else {
        // set the name of to_piece to that of the piece we want to promote to
        // set the bitboard of the piece that's been promoted to

        if(move.is_capture()){
            // move is a promotion with capture move
            capture_piece(to, to_piece_name);
        }

        hm_clock = 0;
        
        promo_piece_name = get_promo_piece(flags, from_piece_colour); // piece that we want to promote to
        bitboards[promo_piece_name] |= set_bit(to);

        add_to_eval(promo_piece_name, to);
    }

    // remove piece from initial square in its bitboard, then set the bitboard
    bitboards[from_piece_name] &= ~set_bit(from);
    sub_from_eval(from_piece_name, from);

    change_turn();    
    generate_position_key(this);

    add_state(move, recent_capture);

    ply++;
}

int Board::get_prev_move(Move& prev_move){
    if(current_state->prev_state != NULL || ep_square){
        prev_move = current_state->prev_move;
        return 0;
    } else {
        return -1;
    }
}

int Board::undo_move(){
    if(current_state->prev_state != NULL){  
        Move prev_move = current_state->prev_move;
        piece_names recent_capture = current_state->recent_capture;

        revert_state();
    
        int from = prev_move.get_from();
        int to = prev_move.get_to();
        int flags = prev_move.get_flags();

        piece_names promo_piece_name;

        piece_names from_piece_name;
        colour from_piece_colour;

        if(!prev_move.is_promo()){
            // remove piece from to square on its bitboard if not a promotion move
            from_piece_name = get_piece_on_square(to);

            assert(from_piece_name != None);

            bitboards[from_piece_name] &= ~set_bit(to);
            sub_from_eval(from_piece_name, to);

            from_piece_colour = (colour)(from_piece_name < 7);

            if(flags == 4){
                uncapture_piece(set_bit(to), recent_capture);
            } else if(flags == 5){
                ep_uncapture(from_piece_colour, to);  
            } else if(flags == 2){
                uncastle_kingside(from, from_piece_colour); 
            } else if(flags == 3){
                uncastle_queenside(from, from_piece_colour); 
            } else if (flags == 1){
                assert(from_piece_name == P || from_piece_name == p);

                hm_clock = current_state->hm_clock;
            
            } else {
                hm_clock--;
            }
            
        } else {
            // remove piece that was promoted to
            promo_piece_name = get_piece_on_square(to); // piece that we wanted to promote to
            bitboards[promo_piece_name] &= ~set_bit(to);
            sub_from_eval(promo_piece_name, to);

            if(prev_move.is_capture()){
                uncapture_piece(set_bit(to), recent_capture);
            } else {
                hm_clock = current_state->hm_clock;
            }
        
            // get correct pawn bitboard
            from_piece_name = (promo_piece_name > 7) ? p : P;
        }

        // put piece at initial square in its bitboard, then set the bitboard
        bitboards[from_piece_name] |= set_bit(from);
        add_to_eval(from_piece_name, from);

        change_turn();

        castling_rights = current_state->castling_rights;
        ep_square = current_state->ep_square;
        hash_key = current_state->hash_key;

        ply--;

        return 0;
    } else {
        std::cout << "Cannot undo, no move has been made yet" << std::endl;
        return -1;
    }
} 

/// Perform en-passant capture
piece_names Board::ep_capture(const colour& pawn_colour, const uint& to){
    piece_names captured_piece_name;
    uint captured_piece_square;

    if(pawn_colour){
        captured_piece_name = p;
        captured_piece_square = to-8;
    }else{
        captured_piece_name = P;
        captured_piece_square = to+8;
    }

    bitboards[captured_piece_name] &= ~set_bit(captured_piece_square);
    sub_from_eval(captured_piece_name, captured_piece_square);

    hm_clock = 0;

    return captured_piece_name;
}

/// Revert en-passant capture
void Board::ep_uncapture(const colour& pawn_colour, const uint& to){
    piece_names captured_piece_name;
    uint captured_piece_square;

    if(pawn_colour){
        captured_piece_name = p;
        captured_piece_square = to-8;
    }else{
        captured_piece_name = P;
        captured_piece_square = to+8;
    }

    bitboards[captured_piece_name] |= set_bit(captured_piece_square);
    add_to_eval(captured_piece_name, captured_piece_square);

    hm_clock = current_state->hm_clock;
}

void Board::castle_kingside(const uint& king_square, const colour& king_colour){
    piece_names rook_type = (king_colour) ? R : r;

    int old_rook_square = king_square - 3, new_rook_square = king_square - 1;

    // remove rook from initial square, put at new square
    bitboards[rook_type] &= ~set_bit(old_rook_square);
    bitboards[rook_type] |= set_bit(new_rook_square);

    add_to_eval(rook_type, new_rook_square);
    sub_from_eval(rook_type, old_rook_square);

    hm_clock++;
}

void Board::uncastle_kingside(const uint& king_square, const colour& king_colour){
    piece_names rook_type = (king_colour) ? R : r;

    int old_rook_square = king_square - 1, new_rook_square = king_square - 3;

    // remove rook from new square, put at initial square
    bitboards[rook_type] |= set_bit(new_rook_square);
    bitboards[rook_type] &= ~set_bit(old_rook_square); 

    add_to_eval(rook_type, new_rook_square);
    sub_from_eval(rook_type, old_rook_square);

    hm_clock--;
}

void Board::castle_queenside(const uint& king_square, const colour& king_colour){
    piece_names rook_type = (king_colour) ? R : r;

    int old_rook_square = king_square + 4, new_rook_square = king_square + 1;

    // remove rook from initial square, put at new square
    bitboards[rook_type] &= ~set_bit(king_square + 4);
    bitboards[rook_type] |= set_bit(king_square + 1);

    add_to_eval(rook_type, new_rook_square);
    sub_from_eval(rook_type, old_rook_square);
    
    hm_clock++;
}   

void Board::uncastle_queenside(const uint& king_square, const colour& king_colour){
    piece_names rook_type = (king_colour) ? R : r;

    int old_rook_square = king_square + 1, new_rook_square = king_square + 4;

    // remove rook from initial square, put at new square
    bitboards[rook_type] |= set_bit(new_rook_square);
    bitboards[rook_type] &= ~set_bit(old_rook_square);

    add_to_eval(rook_type, new_rook_square);
    sub_from_eval(rook_type, old_rook_square);

    hm_clock--;
}   

/// Perform normal capture
void Board::capture_piece(int square, const piece_names& to_piece_name){
    bitboards[to_piece_name] &= ~set_bit(square);

    sub_from_eval(to_piece_name, square);

    hm_clock = 0;
}

/// Revert normal capture
void Board::uncapture_piece(const U64& square_bitboard, piece_names& recent_capture){
    bitboards[recent_capture] |= square_bitboard;

    hm_clock = current_state->hm_clock;  
}

bool Board::has_castling_rights(int flag) const {
    return (castling_rights & flag) != 0;
}   

bool Board::is_occupied(uint square){
    return get_bit(get_entire_bitboard(), square);
}

piece_names Board::get_promo_piece(const uint& flags, const colour& from_piece_colour){
    if(flags == 12 || flags == 8){
        return from_piece_colour ? N : n;
    } else if(flags == 13 || flags == 9){
        return from_piece_colour ? B : b;
    } else if(flags == 14 || flags == 10){
        return from_piece_colour ? R : r;
    } else if(flags == 15 || flags == 11){
        return from_piece_colour ? Q : q;
    } else {
        std::cerr << "Unexpected flag " << flags << " for promotion type move" << std::endl;
        exit(0);
    }
}

void Board::init_from_fen(const std::string fen){
    // parse FEN string
    std::vector<std::string>parts = splitString(removeWhiteSpace(fen), ' ');

    if(parts.size() != 6){
        std::cerr << "Fen string format incorrect" << std::endl;
        exit(0);
    } else {
        init_board_state(parts[0]);
        init_turn(parts[1]);
        init_castling_rights(parts[2]);
        auto maybe_move = init_enpassant_square(parts[3]);
        init_halfmove_clock(parts[4]);
        init_fullmoves(parts[5]);

        psqt_scores[0] = 0, psqt_scores[1] = 0, material[0] = 0, material[1] = 0;
    
        // init material and psqt score
        apply_psqt();
        count_material();
        generate_position_key(this);

        add_state(maybe_move, None);
    }
}

/// Initialise all bitboards and piece lists
void Board::init_board_state(const std::string& board_string){
    int pointer = 0, current_square = 63;
    char c;

    while(current_square >= 0){
        c = board_string[pointer];

        if(isalpha(c)){
            piece_names piece_name = char_to_name[c];

            bitboards[piece_name] |= set_bit(current_square);

            current_square--;

        } else if(isdigit(c)){
            current_square -= (c - '0');
        } 

        pointer++;
    }

}

void Board::init_turn(const std::string& str_turn){
    turn = (colour)abs(str_turn == "w");
}

void Board::init_castling_rights(const std::string& str_castling){
    if(str_castling == "-"){
        castling_rights = 0;
    } else {
        for(auto c : str_castling){
            if(c == 'K'){castling_rights |= 8;}
            else if(c == 'Q'){castling_rights |= 4;}
            else if(c == 'k'){castling_rights |= 2;}
            else if(c == 'q'){castling_rights |= 1;}
            else{std::cout << "What? Castling availability flag in fen string " << str_castling << " is unexpected" << std::endl; exit(0);}
        }
    }
}

/// Given the enpassant square, add the relevant move to move history that would've led to that enpassant square, which will be used to 
/// validate possible enpassant captures by an enemy piece
Move Board::init_enpassant_square(const std::string& str_ep_square){
    if(str_ep_square == "-"){
        ep_square = 0;
        return Move(0,0,0);
    } else {
        ep_square = alg_to_int(str_ep_square);

        if(turn){
            return Move(ep_square-8,ep_square+8,1);
        } else {
            return Move(ep_square+8,ep_square-8,1);
        }
    }
}

void Board::init_halfmove_clock(const std::string& str_hm_clock){
    hm_clock = std::stoi(str_hm_clock);
}

void Board::init_fullmoves(const std::string& str_fullmoves){
    fullmoves = std::stoi(str_fullmoves);
}

piece_names Board::get_piece_on_square(uint square) const {
    for(int i = 0; i < 15; i++){
        if(bitboards[i] & set_bit(square)){return (piece_names)i;}
    }
    
    return None;
}

void Board::view_board(){

    int square_offset;
    std::string turn_to_print, letters_to_print;

    if(turn){
        turn_to_print = "white";
        letters_to_print = "   a  b  c  d  e  f  g  h";
        square_offset = 63;
    } else {
        turn_to_print = "black";
        letters_to_print = "   h  g  f  e  d  c  b  a";
        square_offset = 0;
    }

    std::cout << turn_to_print << " to move" << std::endl; 
    std::cout << "Half move clock: " << hm_clock << std::endl;

    if(ep_square != 0){
        std::cout << "En-passant square: " << int_to_alg(ep_square) << std::endl;
    }

    std::cout << "white psqt " << std::dec << psqt_scores[white] << std::endl;
    std::cout << "black pqst " << std::dec << psqt_scores[black] << std::endl;
    
    std::cout << "  =======================" << std::endl;
                                            
    std::string letter;
    piece_names name;

    for(int i = 0; i < 64; ++i){                
        name = get_piece_on_square(abs(square_offset-i));

        letter = name_to_char(name);

        if((i+1) % 8 == 0){
            std::cout << " " << letter << "|" << std::endl;
        }else if(i % 8 == 0){
            std::cout << 1 + (abs(square_offset-i) / 8) << "| " << letter << " ";
        }else{
            std::cout << " " << letter << " ";
        }
    }
    std::cout << "  ======================="  << std::endl;
    std::cout << letters_to_print << std::endl;
    std::cout << "Key: " << std::uppercase << std::hex << hash_key << std::endl; 
}

U64 Board::get_entire_bitboard() const {
    U64 full_board = 0ULL;

    for(auto bb : bitboards){
        full_board |= bb;
    }
    
    return full_board;
}

/// after a new move has been made, create a new state. Store the new castling rights, the half move clock, the move that led to this state,
/// and the piece if any that got captured when that move was made
void Board::add_state(Move prev_move, piece_names recent_capture){
    current_state = std::make_shared<State>(castling_rights, hm_clock, recent_capture, prev_move, ep_square, hash_key, current_state);
}

/// Make the previous state the current state, then make valid moves the valid moves of that previous state
void Board::revert_state(){
    current_state = current_state->prev_state;
}

void Board::apply_psqt(){
    U64 piece_bitboard;
    int colour_index;
    int square;

    for(int i = 0; i < 15; ++i){
        piece_bitboard = bitboards[i];
        colour_index = abs(i < 7); 

        while(piece_bitboard){
            square = convert_square_to_index(get_lsb(piece_bitboard), colour_index);
            psqt_scores[colour_index] += PSQT[(i % 8)-1][square];

            piece_bitboard &= (piece_bitboard - 1);
        }
    }
}

void Board::count_material(){
    std::vector<piece_names> pieces = {N, P, Q, R, B};

    for(piece_names piece: pieces){
        material[white] += count_set_bits(bitboards[piece]) * get_piece_value[piece];                                      
    }

    pieces = {n, p, q, r, b};

    for(piece_names piece: pieces){
        material[black] += count_set_bits(bitboards[piece]) * get_piece_value[piece-8];
    }
}

void Board::add_to_eval(piece_names piece, int square){
    int colour_index = abs(piece < 7);
    square = convert_square_to_index(square, colour_index);

    // add to psqt scores
    psqt_scores[colour_index] += PSQT[(piece % 8)-1][square];

    // add to material score
    material[colour_index] += get_piece_value[piece];
}


void Board::sub_from_eval(piece_names piece, int square){
    int colour_index = abs(piece < 7);
    square = convert_square_to_index(square, colour_index);
    
    // sub from psqt scores
    psqt_scores[colour_index] -= PSQT[(piece % 8)-1][square];

    // sub from material score
    material[colour_index] -= get_piece_value[piece];
}

void generate_position_key(Board* position){
    U64 occupied = position->get_entire_bitboard();
    int sq, piece_index, ep_square = position->get_ep_square();
    piece_names piece_on_square;
    U64 final_key = 0;

    while(occupied){
        sq = get_lsb(occupied);
        piece_on_square = position->get_piece_on_square(sq);
        piece_index = convert_piece_to_zobrist_index(piece_on_square);
        final_key ^= Piece_keys[piece_index][sq];

        occupied &= (occupied - 1);
    }

    if(position->get_turn()){
        final_key ^= Turn_key;
    }

    if(ep_square != 0){
        final_key ^= Piece_keys[None][ep_square];
    }

    final_key ^= Castle_key[position->get_castling_rights()];

    position->hash_key = final_key;
}

/// @brief Store a move into the PV table
/// @param position 
/// @param move 
void store_pv_move(Board* position, uint16_t move, int eval){
    int index = position->hash_key % position->pv_table.num_of_entries;

    assert((0 <= index) && (index <= position->pv_table.num_of_entries - 1));

    if(!position->pv_table.collides(index, position->hash_key)){
        position->pv_table.pv_entries[index].move = move;
        position->pv_table.pv_entries[index].eval = eval;
        position->pv_table.pv_entries[index].hash_key = position->hash_key;
    } else if (debug) {
        std::cout << "PV entry for position with key " << position->hash_key << " is taken by " << position->pv_table.pv_entries[index].hash_key << std::endl;
    }
}

/// @brief Get the move from pv table if this position has been stored
/// @param *position 
uint16_t probe_pv_move(Board* position){
    int index = position->hash_key % position->pv_table.num_of_entries;

    if(position->pv_table.pv_entries[index].hash_key == position->hash_key){
        return position->pv_table.pv_entries[index].move;
    } else {
        return 0;
    }
}

/// @brief Get evaluation from pv table if this position has been stored
/// @param *position 
std::optional<int> probe_pv_score(Board* position){
    int index = position->hash_key % position->pv_table.num_of_entries;

    if(position->pv_table.pv_entries[index].hash_key == position->hash_key){
        return position->pv_table.pv_entries[index].eval;
    } else {
        return std::nullopt;
    }
}