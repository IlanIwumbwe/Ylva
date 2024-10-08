#include "../headers/board.h"

U64 bitboards[12] = {0ULL};
info board_infos[MAX_SEARCH_DEPTH];
info* board_info = board_infos;  // start by pointing to first element in board infos
piece board[64];

// zobrist keys
U64 piece_zobrist_keys[13][64];
U64 turn_key;
U64 castling_key[16];

void init_hash_keys(void){
    int i, j;

    for(i = 0; i < 13; ++i){
        for(j = 0; j < 64; ++j){
            piece_zobrist_keys[i][j] = RAND64;
        }
    }

    turn_key = RAND64;

    for(i = 0; i < 16; ++i){
        castling_key[i] = RAND64;
    }

}

void generate_hash(){

    U64 occupied = bitboards[P] | bitboards[K] | bitboards[N] | bitboards[B] | bitboards[R] | bitboards[Q] | 
            bitboards[p] | bitboards[k] | bitboards[n] | bitboards[b] | bitboards[r] | bitboards[q];
    square sq;

    board_info->hash = 0ULL;

    while(occupied){
        sq = get_lsb(occupied);
        occupied &= (occupied - 1);

        board_info->hash ^= piece_zobrist_keys[piece_on_square(sq)][sq];
    }

    if(board_info->ep_square != s_none){
        board_info->hash ^= piece_zobrist_keys[p_none][board_info->ep_square];
    }

    if(board_info->s == BLACK){
        board_info->hash ^= turn_key;
    }

    board_info->hash ^= castling_key[board_info->castling_rights & 0xf];
}

void modify_hash_by_occupancy(info* info_n, piece p, square sq){
    info_n->hash ^= piece_zobrist_keys[p][sq];
}

void modify_hash_by_castling_rights(info* info_n, U16 old_castling_rights){
    info_n->hash ^= old_castling_rights;
    info_n->hash ^= info_n->castling_rights;
}   

/// @brief Populate bitboards from fen
/// @param board_string 
void setup_bitboards(const char* fen){
    int pointer = 0, current_square = 63;
    char c;
    piece piece = p_none;
    
    memset(bitboards, 0, sizeof(bitboards));

    for(int i = 0; i < 64; ++i){board[i] = p_none;}

    while(current_square >= 0){
        c = fen[pointer];

        if(isalpha(c)){
            piece = char_to_piece(c);
        
            if(piece != p_none){
                bitboards[piece] |= set_bit(current_square);
                board[current_square] = piece; 
            }

            current_square--;

        } else if(isdigit(c)){
            current_square -= (c - '0');
        }

        pointer++;
    }
}

/// @brief receives a fen string and a pointer to bitboards array. init state
/// @param fen_string 
/// @param bitboards 
void setup_state_from_fen(char* fen_string){ 
    char* end;
    char* copy = strdup(fen_string);

    char* t = strtok(copy, " ");

    setup_bitboards(t);

    board_info->s = (*strtok(NULL, " ") == 'w') ? WHITE : BLACK;

    char* castling_rights = strtok(NULL, " ");
    char c;

    while((c = *castling_rights++)){
        switch(c){
            case 'K': board_info->castling_rights |= K_castle; break;
            case 'Q': board_info->castling_rights |= Q_castle; break;
            case 'k': board_info->castling_rights |= k_castle; break;
            case 'q': board_info->castling_rights |= q_castle; break;
        }
    }

    board_info->ep_square = char_to_square(strtok(NULL, " "));
    board_info->move = 0;

    if(board_info->ep_square != s_none){
        // make previous move a double pawn push that would've led to this en-passant square
        
        castling_and_enpassant_info cep = cep_info[board_info->s];

        board_info->move = (1 << 12) | ((board_info->ep_square - cep.ep_sq_offset) << 6) | (board_info->ep_square + cep.ep_sq_offset);
    }


    board_info->ply = strtol(strtok(NULL, " "), &end, 10);
    board_info->moves = strtol(strtok(NULL, " "), &end, 10);
    board_info->captured_piece = p_none;

    generate_hash();
}

piece piece_on_square(square sq){

    return board[sq];
}

void print_board(void){
    piece p;
    char c; 

    printf("castling rights flag: %d\n", board_info->castling_rights);
    printf("ply: %d\n", board_info->ply);
    printf("moves: %d\n", board_info->moves);
    printf("previous move: ");
    print_move(board_info->move);
    printf("move type: %x\n", move_type(board_info->move));
    printf("turn: %s\n", (board_info->s) ? "b" : "w");

    printf("----------------\n");
    for(int i = 63; i >= 0; i--){
        p = piece_on_square(i);
        
        c = (p == p_none) ? '.' : char_pieces[p];
    
        printf("%c ", c);  

        if((i % 8) == 0){
            printf("|%d\n", (i/8) + 1);
        }
    }
    printf("----------------\n");
    printf("a b c d e f g h\n\n");

    printf("Key: %lx \n", board_info->hash);
}

