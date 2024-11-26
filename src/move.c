#include "../headers/move.h"

/// @brief Manipulate bitboards to make move
/// @param bitboards 
/// @param move 
void make_move(board_state* state, const U16 move){
    history* info_n = state->data + 1; // point to next available memory location in array

    info_n->castling_rights = state->data->castling_rights;
    info_n->ep_square = s_none;
    info_n->hash = state->data->hash ^ zi.turn_key;
    info_n->occupied = state->data->occupied;
    info_n->fifty_move = state->data->fifty_move;
    info_n->ply = state->data->ply;
    info_n->hisply = state->data->hisply;
    info_n->moves = state->data->moves;
    
    memcpy(info_n->eval, state->data->eval, sizeof(state->data->eval));

    side s = state->data->s;

    square s_from = move_from_square(move);
    square s_to = move_to_square(move);
    square ep = state->data->ep_square;
    int m_type = move_type(move);

    piece p_from = state->board[s_from];
    piece p_to = state->board[s_to];

    U64 val = -(ep != s_none);
    info_n->hash ^= (val & zi.piece_zobrist_keys[p_none][ep]);

    assert(p_from != p_none);

    info_n->fifty_move += 1; // fifty move counter increments by defualt, if not reset, incremented value is kept

    if(p_from == P || p_from == p){info_n->fifty_move = 0;} // pawn moves reset fifty move counter

    // take care of normal captures, ep capture done separately
    if(p_to != p_none){
        state->bitboards[p_to] &= ~set_bit(s_to);
        info_n->captured_piece = p_to;
        info_n->fifty_move = 0;
    }

    if(p_to != p_none) info_n->eval[s] -= PIECE_VALUES[p_to][FLIP[1-s][s_to]];

    castling_and_enpassant_info opp_cep = cep_info[1-s];
    castling_and_enpassant_info ally_cep = cep_info[s];

    /* 
    rook was captured, king moved, rook moved 
    remove relevant casting rights
    */
    if(p_to == opp_cep.rook_to_move){
        if (s_to == opp_cep.rook_kingside_sq){
            info_n->castling_rights &= ~opp_cep.kcr;
        } else if (s_to == opp_cep.rook_queenside_sq){
            info_n->castling_rights &= ~opp_cep.qcr;
        }
    } else if (p_from == ally_cep.ally_king){
        info_n->castling_rights &= ~(ally_cep.kcr | ally_cep.qcr);

    } else if ((p_from == ally_cep.rook_to_move) && (s_from == ally_cep.rook_kingside_sq)){
        info_n->castling_rights &= ~ally_cep.kcr;

    } else if ((p_from == ally_cep.rook_to_move) && (s_from == ally_cep.rook_queenside_sq)){
        info_n->castling_rights &= ~ally_cep.qcr;
    }
    
    state->bitboards[p_from] &= ~set_bit(s_from);
    info_n->occupied &= ~set_bit(s_from);assert(p_from != p_none);
    state->board[s_from] = p_none;

    info_n->hash ^= zi.piece_zobrist_keys[p_from][s_from];

    // promotion move or not
    if(m_type <= 5){

        if(m_type == 1){
            
            info_n->ep_square = s_to + ally_cep.ep_sq_offset;
            info_n->hash ^= zi.piece_zobrist_keys[p_none][info_n->ep_square];
            info_n->fifty_move = 0;

        } else if(m_type == 2){
            // kingside castle

            info_n->castling_rights &= ~(ally_cep.kcr | ally_cep.qcr);

            state->bitboards[ally_cep.rook_to_move] &= ~set_bit(s_to - 1);
            state->bitboards[ally_cep.rook_to_move] |= set_bit(s_from - 1);

            state->board[s_to - 1] = p_none;
            state->board[s_from - 1] = ally_cep.rook_to_move;

            info_n->occupied &= ~set_bit(s_to - 1);
            info_n->occupied |= set_bit(s_from - 1);
            info_n->hash ^= zi.piece_zobrist_keys[ally_cep.rook_to_move][s_to - 1];
            info_n->hash ^= zi.piece_zobrist_keys[ally_cep.rook_to_move][s_from - 1];
 
            modify_hash_by_castling_rights(state, state->data->castling_rights);
            
        } else if (m_type == 3){
            // queenside castle
            info_n->castling_rights &= ~(ally_cep.kcr | ally_cep.qcr);

            state->bitboards[ally_cep.rook_to_move] &= ~set_bit(s_to + 2);
            state->bitboards[ally_cep.rook_to_move] |= set_bit(s_from + 1);

            info_n->occupied &= ~set_bit(s_to + 2);
            info_n->occupied |= set_bit(s_from + 1);

            state->board[s_to + 2] = p_none;
            state->board[s_from + 1] = ally_cep.rook_to_move;

            info_n->hash ^= zi.piece_zobrist_keys[ally_cep.rook_to_move][s_to + 2];
            info_n->hash ^= zi.piece_zobrist_keys[ally_cep.rook_to_move][s_from + 1];

            modify_hash_by_castling_rights(state, state->data->castling_rights);

        } else if (m_type == 5){
            // ep capture
            state->bitboards[ally_cep.ep_pawn] &= ~set_bit(s_to + ally_cep.ep_sq_offset);
            info_n->occupied &= ~set_bit(s_to + ally_cep.ep_sq_offset);

            info_n->captured_piece = ally_cep.ep_pawn;
            state->board[s_to + ally_cep.ep_sq_offset] = p_none;

            info_n->hash ^= zi.piece_zobrist_keys[ally_cep.ep_pawn][s_to + ally_cep.ep_sq_offset];
            info_n->eval[s] -= PIECE_VALUES[ally_cep.ep_pawn][FLIP[1-s][s_to + ally_cep.ep_sq_offset]];
            info_n->fifty_move = 0;
        } 

    } else {
        int offset = (s == BLACK) ? 8 : 2;

        info_n->eval[s] -= PIECE_VALUES[p_from][FLIP[s][s_from]];
        info_n->fifty_move = 0; // promotion moves involve pawn movement so should reset fifty move counter
        p_from = (m_type & 0x3) + offset;
    }

    state->bitboards[p_from] |= set_bit(s_to);
    state->board[s_to] = p_from;
    info_n->eval[s] += PIECE_VALUES[p_from][FLIP[s][s_to]];
    info_n->occupied |= set_bit(s_to);
    info_n->hash ^= zi.piece_zobrist_keys[p_from][s_to];

    info_n->s = 1 - s;
    info_n->ply += 1;
    info_n->hisply += 1;
    info_n->moves += (s == BLACK);
    info_n->move = move;

    state->data = info_n; // move board info pointer to point to next available memory location
}

/// @brief Undoes most recently made move
/// @param move 
void undo_move(board_state* state){
    U16 move = state->data->move;

    assert(move != 0U);

    square s_from = move_from_square(move);
    square s_to = move_to_square(move);
    int m_type = move_type(move);

    piece p_from = state->board[s_to];

    if((m_type != 5) && (m_type & 0x4)){
        state->bitboards[state->data->captured_piece] |= set_bit(s_to);
        state->board[s_to] = state->data->captured_piece;

    } else {
        state->board[s_to] = p_none;
    }

    state->bitboards[p_from] &= ~set_bit(s_to);

    assert(p_from != p_none);

    // promotion move or not
    if(m_type <= 5){
        castling_and_enpassant_info cep = cep_info[1 - state->data->s];

        if(m_type == 5){
            //castling_and_enpassant_info cep = cep_info[board_info->s];

            state->bitboards[state->data->captured_piece] |= set_bit(s_to + cep.ep_sq_offset); 
            state->board[s_to + cep.ep_sq_offset] = state->data->captured_piece;

        } else if(m_type == 2){
            // kingside castle
            state->bitboards[cep.rook_to_move] |= set_bit(s_to - 1);
            state->bitboards[cep.rook_to_move] &= ~set_bit(s_from - 1);

            state->board[s_to - 1] = cep.rook_to_move;
            state->board[s_from - 1] = p_none;
            
        } else if (m_type == 3){
            // queenside castle
            state->bitboards[cep.rook_to_move] |= set_bit(s_to + 2);
            state->bitboards[cep.rook_to_move] &= ~set_bit(s_from + 1);

            state->board[s_to + 2] = cep.rook_to_move;
            state->board[s_from + 1] = p_none;
        }

    } else {        
        p_from = (p_from > 5) ? p : P;
    }

    state->bitboards[p_from] |= set_bit(s_from);
    state->board[s_from] = p_from;

    state->data -= 1;  // move board info pointer to point to previous board metadata
}