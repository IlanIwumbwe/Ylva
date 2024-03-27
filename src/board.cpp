#include <assert.h>
#include "board.h"

int State::state_id = 0;

Board::Board (const std::string& _fen){
    // parse FEN string
    std::vector<std::string>parts = splitString((_fen == "") ? STARTING_FEN : removeWhiteSpace(_fen), ' ');
    
    init_from_fen(parts);
}

/// Make move given as input on the board
void Board::make_move(const Move& move){
    // std::cout << "making move " << move << std::endl;

    const auto from = move.get_from();
    const auto to = move.get_to();
    const auto flags = move.get_flags();

    const auto from_piece_name = get_piece_on_square(from);
    const auto to_piece_name = get_piece_on_square(to);

    auto from_piece_colour = get_piece_colour(from_piece_name);

    if(from_piece_name == None){
        std::cout << int_to_alg(from) << std::endl;

        view_board();
    }

    assert(from_piece_name != None);

    U64 from_piece_bitboard = get_piece_bitboard(from_piece_name);
    piece_names promo_piece_name;
    U64 promotion_piece_bitboard;

    auto recent_capture = to_piece_name;

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

    if(!move.is_promo()){
        // place piece at to square if not promotion move
        from_piece_bitboard |= set_bit(to);
        
        if(flags == 4){
            capture_piece(to_piece_name, set_bit(to));
        } else if(flags == 5){
            recent_capture = ep_capture(from_piece_colour, to);
        } else if(flags == 2){
            castle_kingside(from, from_piece_colour);
        } else if(flags == 3){
            castle_queenside(from, from_piece_colour);
        } else {
            // quiet moves and double pawn pushes
            if(from_piece_name == P || from_piece_name == p){hm_clock = 0;} // pawn advance
            else{hm_clock++;}  // other quiet moves
        }
    
    } else {
        // set the name of to_piece to that of the piece we want to promote to
        // set the bitboard of the piece that's been promoted to

        if(move.is_capture()){
            // move is a promotion with capture move
            capture_piece(to_piece_name, set_bit(to));
        } 
        hm_clock = 0;
        
        promo_piece_name = get_promo_piece(flags, from_piece_colour); // piece that we want to promote to
        promotion_piece_bitboard = get_piece_bitboard(promo_piece_name);
        promotion_piece_bitboard |= set_bit(to);
        set_piece_bitboard(promo_piece_name, promotion_piece_bitboard);
    }

    // remove piece from initial square in its bitboard, then set the bitboard
    from_piece_bitboard &= ~set_bit(from);
    set_piece_bitboard(from_piece_name, from_piece_bitboard);

    add_state(move, recent_capture);
    
    change_turn();
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

        // std::cout << "undoing move " << prev_move << std::endl;

        revert_state();
    
        const auto from = prev_move.get_from();
        const auto to = prev_move.get_to();
        const auto flags = prev_move.get_flags();

        piece_names promo_piece_name;
        U64 promotion_piece_bitboard;

        piece_names from_piece_name;
        colour from_piece_colour;
        U64 from_piece_bitboard;

        if(!prev_move.is_promo()){
            // remove piece from to square on its bitboard if not a promotion move
            from_piece_name = get_piece_on_square(to);

            assert(from_piece_name != None);

            from_piece_bitboard = get_piece_bitboard(from_piece_name) & ~set_bit(to);
            from_piece_colour = get_piece_colour(from_piece_name);

            if(flags == 4){
                uncapture_piece(set_bit(to), recent_capture);
            } else if(flags == 5){
                ep_uncapture(from_piece_colour, to);  
            } else if(flags == 2){
                uncastle_kingside(from, from_piece_colour); 
            } else if(flags == 3){
                uncastle_queenside(from, from_piece_colour); 
            } else {
                // quiet moves and double pawn pushes
                if(from_piece_name == P || from_piece_name == p){hm_clock = current_state->hm_clock;} // pawn advance
                else{hm_clock--;}  // other quiet moves
            }

        } else {
            // remove piece that was promoted to
            promo_piece_name = get_piece_on_square(to); // piece that we wanted to promote to
            promotion_piece_bitboard = get_piece_bitboard(promo_piece_name) & ~set_bit(to);
            set_piece_bitboard(promo_piece_name, promotion_piece_bitboard);

            if(prev_move.is_capture()){
                uncapture_piece(set_bit(to), recent_capture);
            } else {
                hm_clock = current_state->hm_clock;
            }
        
            // get correct pawn bitboard
            from_piece_name = get_piece_colour(promo_piece_name) ? p : P;
            from_piece_bitboard = get_piece_bitboard(from_piece_name);
        }

        // put piece at initial square in its bitboard, then set the bitboard
        from_piece_bitboard |= set_bit(from);
        set_piece_bitboard(from_piece_name, from_piece_bitboard);

        change_turn();

        castling_rights = current_state->castling_rights;

        return 0;
    } else {
        std::cout << "Cannot undo, no move has been made yet" << std::endl;
        return -1;
    }
} 

/// Perform en-passant capture
piece_names Board::ep_capture(const colour& pawn_colour, const uint& to){
    U64 captured_piece_bitboard = 0;
    piece_names captured_piece_name;
    uint captured_piece_square;

    if(pawn_colour == WHITE){
        captured_piece_name = p;
        captured_piece_square = to-8;
    }else{
        captured_piece_name = P;
        captured_piece_square = to+8;
    }

    captured_piece_bitboard = get_piece_bitboard(captured_piece_name);
    captured_piece_bitboard &= ~set_bit(captured_piece_square);
    set_piece_bitboard(captured_piece_name, captured_piece_bitboard);

    hm_clock = 0;

    return captured_piece_name;
}

/// Revert en-passant capture
void Board::ep_uncapture(const colour& pawn_colour, const uint& to){
    U64 captured_piece_bitboard = 0;
    piece_names captured_piece_name;
    uint captured_piece_square;

    if(pawn_colour == WHITE){
        captured_piece_name = p;
        captured_piece_square = to-8;
    }else{
        captured_piece_name = P;
        captured_piece_square = to+8;
    }

    captured_piece_bitboard = get_piece_bitboard(captured_piece_name);
    captured_piece_bitboard |= set_bit(captured_piece_square);
    set_piece_bitboard(captured_piece_name, captured_piece_bitboard);

    hm_clock = current_state->hm_clock;
}

void Board::castle_kingside(const uint& king_square, const colour& king_colour){
    auto rook_type = (king_colour) ? r : R;
    U64 rook_bitboard = get_piece_bitboard(rook_type);

    // remove rook from initial square, put at new square
    rook_bitboard &= ~set_bit(king_square - 3);
    rook_bitboard |= set_bit(king_square - 1);
    set_piece_bitboard(rook_type, rook_bitboard); 
    hm_clock++;
}

void Board::uncastle_kingside(const uint& king_square, const colour& king_colour){
    auto rook_type = (king_colour) ? r : R;
    U64 rook_bitboard = get_piece_bitboard(rook_type);

    // remove rook from new square, put at initial square
    rook_bitboard |= set_bit(king_square - 3);
    rook_bitboard &= ~set_bit(king_square - 1);
    set_piece_bitboard(rook_type, rook_bitboard); 
    hm_clock--;
}

void Board::castle_queenside(const uint& king_square, const colour& king_colour){
    auto rook_type = (king_colour) ? r : R;
    U64 rook_bitboard = get_piece_bitboard(rook_type);

    // remove rook from initial square, put at new square
    rook_bitboard &= ~set_bit(king_square + 4);
    rook_bitboard |= set_bit(king_square + 1);
    set_piece_bitboard(rook_type, rook_bitboard); 
    hm_clock++;
}   

void Board::uncastle_queenside(const uint& king_square, const colour& king_colour){
    auto rook_type = (king_colour) ? r : R;
    U64 rook_bitboard = get_piece_bitboard(rook_type);

    // remove rook from initial square, put at new square
    rook_bitboard |= set_bit(king_square + 4);
    rook_bitboard &= ~set_bit(king_square + 1);
    set_piece_bitboard(rook_type, rook_bitboard); 
    hm_clock--;
}   

/// Perform normal capture
void Board::capture_piece(const piece_names& to_piece_name, const U64& square_bitboard){
    auto captured_piece_bitboard = get_piece_bitboard(to_piece_name);
    captured_piece_bitboard &= ~square_bitboard;
    set_piece_bitboard(to_piece_name, captured_piece_bitboard);
    hm_clock = 0;
}

/// Revert normal capture
void Board::uncapture_piece(const U64& square_bitboard, piece_names& recent_capture){
    auto captured_piece_bitboard = get_piece_bitboard(recent_capture);
    captured_piece_bitboard |= square_bitboard;
    set_piece_bitboard(recent_capture, captured_piece_bitboard);

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
        return (from_piece_colour == WHITE) ? N : n;
    } else if(flags == 13 || flags == 9){
        return (from_piece_colour == WHITE) ? B : b;
    } else if(flags == 14 || flags == 10){
        return (from_piece_colour == WHITE) ? R : r;
    } else if(flags == 15 || flags == 11){
        return (from_piece_colour == WHITE) ? Q : q;
    } else {
        std::cerr << "Unexpected flag " << flags << " for promotion type move" << std::endl;
        exit(0);
    }
}

void Board::init_from_fen(const std::vector<std::string>& parts){
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

            set_piece_bitboard(piece_name, get_piece_bitboard(piece_name) | set_bit(current_square));

            current_square--;

        } else if(isdigit(c)){
            current_square -= (c - '0');
        } 

        pointer++;
    }

}

void Board::init_turn(const std::string& str_turn){
    if(str_turn == "w"){
        turn = WHITE;
    } else {
        turn = BLACK;
    }
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

        if(turn == WHITE){
            return Move(ep_square+8,ep_square-8,1);
        } else {
            return Move(ep_square-8,ep_square+8,1);
        }
    }
}

void Board::init_halfmove_clock(const std::string& str_hm_clock){
    hm_clock = std::stoi(str_hm_clock);
}

void Board::init_fullmoves(const std::string& str_fullmoves){
    fullmoves = std::stoi(str_fullmoves);
}

int Board::get_fullmoves(){
    return floor(current_state->state_id / 2) + fullmoves;
}

piece_names Board::get_piece_on_square(int square) const {
    for(auto p: bitboards){
        if((p.second & set_bit(square)) != 0){return p.first;}
    }
    
    return None;
}

void Board::view_board(){
    std::cout << ((turn == WHITE) ? "white" : "black") << " to move" << std::endl; 
    std::cout << "Half move clock: " << hm_clock << std::endl;
    std::cout << "  =======================" << std::endl;
                                            
    char letter;
    piece_names name;

    for(int i = 0; i < 64; ++i){                
        name = get_piece_on_square(63-i);

        letter = name_to_char(name);

        if((i+1) % 8 == 0){
            std::cout << " " << letter << "| \n";
        }else if(i % 8 == 0){
            std::cout << 8-(i / 8) << "| " << letter << " ";
        }else{
            std::cout << " " << letter << " ";
        }
    }
    std::cout << "  ======================="  << std::endl;
    std::cout << "   a  b  c  d  e  f  g  h" << std::endl;
}

bool Board::move_is_valid(Move& move){
    for(auto v_move : valid_moves){
        if(v_move == move){return true;}
    }
    
    return false;
}

void Board::clear_valid_moves(){
    valid_moves.clear();
}

inline void Board::set_piece_bitboard(const piece_names& piece_name, const U64& bitboard) {
    assert(piece_name != None);
    bitboards[piece_name] = bitboard;
}

inline colour Board::get_piece_colour(const piece_names& piece_name){
    return (colour)(piece_name > 7);
}

U64 Board::get_entire_bitboard() const {
    U64 full_board = 0;

    for(auto p : bitboards){
        full_board |= p.second;
    }
    
    return full_board;
}

void Board::add_valid_move(const Move& valid_move){
    valid_moves.push_back(valid_move);
}

std::vector<Move> Board::get_valid_moves(){    
    return valid_moves;
}

/// after a new move has been made, create a new state. Store the new castling rights, the half move clock, the move that led to this state,
/// and the piece if any that got captured when that move was made
void Board::add_state(Move prev_move, piece_names recent_capture){
    auto new_current = std::make_shared<State>(castling_rights, hm_clock, recent_capture, prev_move);
        
    new_current->prev_state = current_state;
    current_state = new_current;
}

/// Make the previous state the current state, then make valid moves the valid moves of that previous state
void Board::revert_state(){
    current_state = current_state->prev_state;
}

