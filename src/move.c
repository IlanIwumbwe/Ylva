#include "../headers/move.h"

/// @brief Manipulate bitboards to make move
/// @param bitboards 
/// @param move 
void make_move(const U16 move){
    info* info_n = board_info + 1; // point to next available memory location in array

    info_n->castling_rights = board_info->castling_rights;
    info_n->ep_square = s_none;
    info_n->hash = board_info->hash ^ turn_key;
    info_n->occupied = board_info->occupied;
    
    memcpy(info_n->material, board_info->material, sizeof(board_info->material));

    side s = board_info->s;

    square s_from = move_from_square(move);
    square s_to = move_to_square(move);
    square ep = board_info->ep_square;
    int m_type = move_type(move);

    piece p_from = piece_on_square(s_from);
    piece p_to = piece_on_square(s_to);

    U64 val = -(ep != s_none);
    info_n->hash ^= (val & piece_zobrist_keys[p_none][ep]);

    // take care of possible captures, ep capture done separately
    bitboards[p_to] &= ~set_bit(s_to);
    info_n->captured_piece = p_to;
    info_n->material[s] -= piece_values[p_to];

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
    
    assert(p_from != p_none);

    bitboards[p_from] &= ~set_bit(s_from);
    info_n->occupied &= ~set_bit(s_from);
    board[s_from] = p_none;

    info_n->hash ^= piece_zobrist_keys[p_from][s_from];

    // promotion move or not
    if(m_type <= 5){

        if(m_type == 1){
            
            info_n->ep_square = s_to + ally_cep.ep_sq_offset;
            info_n->hash ^= piece_zobrist_keys[p_none][info_n->ep_square];

        } else if(m_type == 2){
            // kingside castle

            info_n->castling_rights &= ~(ally_cep.kcr | ally_cep.qcr);

            bitboards[ally_cep.rook_to_move] &= ~set_bit(s_to - 1);
            bitboards[ally_cep.rook_to_move] |= set_bit(s_from - 1);

            board[s_to - 1] = p_none;
            board[s_from - 1] = ally_cep.rook_to_move;

            info_n->occupied &= ~set_bit(s_to - 1);
            info_n->occupied |= set_bit(s_from - 1);
            info_n->hash ^= piece_zobrist_keys[ally_cep.rook_to_move][s_to - 1];
            info_n->hash ^= piece_zobrist_keys[ally_cep.rook_to_move][s_from - 1];
 
            modify_hash_by_castling_rights(info_n, board_info->castling_rights);
            
        } else if (m_type == 3){
            // queenside castle
            info_n->castling_rights &= ~(ally_cep.kcr | ally_cep.qcr);

            bitboards[ally_cep.rook_to_move] &= ~set_bit(s_to + 2);
            bitboards[ally_cep.rook_to_move] |= set_bit(s_from + 1);

            info_n->occupied &= ~set_bit(s_to + 2);
            info_n->occupied |= set_bit(s_from + 1);

            board[s_to + 2] = p_none;
            board[s_from + 1] = ally_cep.rook_to_move;

            info_n->hash ^= piece_zobrist_keys[ally_cep.rook_to_move][s_to + 2];
            info_n->hash ^= piece_zobrist_keys[ally_cep.rook_to_move][s_from + 1];

            modify_hash_by_castling_rights(info_n, board_info->castling_rights);

        } else if (m_type == 5){
            // ep capture
            bitboards[ally_cep.ep_pawn] &= ~set_bit(s_to + ally_cep.ep_sq_offset);
            info_n->occupied &= ~set_bit(s_to + ally_cep.ep_sq_offset);

            info_n->captured_piece = ally_cep.ep_pawn;
            board[s_to + ally_cep.ep_sq_offset] = p_none;

            info_n->hash ^= piece_zobrist_keys[ally_cep.ep_pawn][s_to + ally_cep.ep_sq_offset];

            info_n->material[s] -= piece_values[ally_cep.ep_pawn];
        } 

    } else {
        int offset = (s == BLACK) ? 8 : 2;

        info_n->material[s] -= piece_values[p_from];

        p_from = (m_type & 0x3) + offset;

        info_n->material[s] += piece_values[p_from];

    }

    bitboards[p_from] |= set_bit(s_to);
    board[s_to] = p_from;
    info_n->occupied |= set_bit(s_to);
    info_n->hash ^= piece_zobrist_keys[p_from][s_to];

    info_n->s = 1 - s;
    info_n->ply = board_info->ply + 1;
    info_n->hisply = board_info->hisply + 1;
    info_n->moves = board_info->moves + (s == BLACK);
    info_n->move = move;

    board_info = info_n; // move board info pointer to point to next available memory location
}

/// @brief Undoes most recently made move
/// @param move 
void undo_move(){
    U16 move = board_info->move;

    assert(move != 0U);

    square s_from = move_from_square(move);
    square s_to = move_to_square(move);
    int m_type = move_type(move);

    piece p_from = piece_on_square(s_to);

    if((m_type != 5) && (m_type & 0x4)){
        bitboards[board_info->captured_piece] |= set_bit(s_to);
        board[s_to] = board_info->captured_piece;

    } else {
        board[s_to] = p_none;
    }

    bitboards[p_from] &= ~set_bit(s_to);

    assert(p_from != p_none);

    // promotion move or not
    if(m_type <= 5){
        castling_and_enpassant_info cep = cep_info[1 - board_info->s];

        if(m_type == 5){
            //castling_and_enpassant_info cep = cep_info[board_info->s];

            bitboards[board_info->captured_piece] |= set_bit(s_to + cep.ep_sq_offset); 
            board[s_to + cep.ep_sq_offset] = board_info->captured_piece;

        } else if(m_type == 2){
            // kingside castle
            bitboards[cep.rook_to_move] |= set_bit(s_to - 1);
            bitboards[cep.rook_to_move] &= ~set_bit(s_from - 1);

            board[s_to - 1] = cep.rook_to_move;
            board[s_from - 1] = p_none;
            
        } else if (m_type == 3){
            // queenside castle
            bitboards[cep.rook_to_move] |= set_bit(s_to + 2);
            bitboards[cep.rook_to_move] &= ~set_bit(s_from + 1);

            board[s_to + 2] = cep.rook_to_move;
            board[s_from + 1] = p_none;
        }

    } else {        
        p_from = (p_from > 5) ? p : P;
    }

    bitboards[p_from] |= set_bit(s_from);
    board[s_from] = p_from;

    board_info -= 1;  // move board info pointer to point to previous board information
}