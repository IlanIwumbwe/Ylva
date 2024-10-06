#include "../headers/board.h"

U64 bitboards[12] = {0ULL};
info board_infos[MAX_SEARCH_DEPTH];
info* board_info = board_infos;  // start by pointing to first element in board infos
piece board[64] = {[0 ... 63] = p_none};

/// @brief Populate bitboards from fen
/// @param board_string 
void setup_bitboards(const char* fen){
    int pointer = 0, current_square = 63;
    char c;
    piece piece = p_none;

    while(current_square >= 0){
        c = fen[pointer];

        if(isalpha(c)){
            piece = char_to_piece(c);
        
            if(piece != p_none){
                bitboards[piece] |= set_bit(current_square);
                board_info->occupied |= set_bit(current_square);   
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

    board_info->turn = (*strtok(NULL, " ") == 'w') ? 0 : 1;

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
    board_info->ply = strtol(strtok(NULL, " "), &end, 10);
    board_info->moves = strtol(strtok(NULL, " "), &end, 10);
    board_info->move = 0;
    board_info->captured_piece = p_none;
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
    printf("previous move: %x\n", board_info->move);
    printf("turn: %s\n", (board_info->turn) ? "b" : "w");

    printf("----------------\n");
    for(int i = 63; i >= 0; --i){
        p = piece_on_square(i);
        
        c = (p == p_none) ? '.' : char_pieces[p];
    
        printf("%c ", c);  

        if((i % 8) == 0){
            printf("|%d\n", (i/8) + 1);
        }
    }
    printf("----------------\n");
    printf("a b c d e f g h\n");
}

