#include "../headers/board.h"

/// @brief Initialise pv table 
/// @param pvt pointer to pv table
/// @param capacity number of entries to put into pv table
void init_pv(board_state* state, size_t capacity){
    if(capacity == 0){
        fprintf(stderr, "Cannot initialise array with capacity of 0 bytes!");
        exit(-1);
    }

    state->pvt.table = (pv_entry*) malloc(sizeof(pv_entry) * capacity);

    if(state->pvt.table != NULL){  
        state->pvt.capacity = capacity;
        reset_pv_entries(state);
    } else {
        fprintf(stderr, "Allocation of memory for dynamic array failed!");
        exit(-1);
    }
}

/// @brief Set all pv entries in the table to 0
/// @param pvt 
void reset_pv_entries(board_state* state){
    for(size_t i = 0; i < state->pvt.capacity; ++i){
        state->pvt.table[i].key = 0ULL;
        state->pvt.table[i].move = 0;
    }
}

/// @brief create an entry using the key and move given, and store it into the pv table
/// @param pvt 
/// @param key 
/// @param move 
void store_pv_entry(board_state* state, U16 move){
    assert(move != 0);

    U64 key = state->data->hash;
    int index = key % state->pvt.capacity;

    state->pvt.table[index].key = key;  
    state->pvt.table[index].move = move;    
} 

/// @brief index into pv table using key, if a move has been stored for this position in the pv table, return it, else return 0
/// @param pvt 
/// @param key 
/// @return 
U16 probe_pv_table(board_state* state){
    U64 key = state->data->hash;
    int index = key % state->pvt.capacity;

    if(state->pvt.table[index].key == key){
        return state->pvt.table[index].move;
    }

    return 0;
}

void free_pv(pv_table* pvt){
    free(pvt);
}

void clear_pv_array(U16* array){
    for(int i = 0; i < MAX_SEARCH_DEPTH; ++i){
        array[i] = 0;
    }
}

void init_eval(board_state* state){
    piece pi;

    for(int sq = 0; sq < 64; ++sq){
        pi = state->board[sq];

        if(pi != p_none){

            if(pi < 6){
                // white
                state->data->eval[WHITE] += PIECE_VALUES[pi][FLIP[WHITE][sq]];
            } else {
                // black
                state->data->eval[BLACK] += PIECE_VALUES[pi][FLIP[BLACK][sq]];
            }
        }
    }
}

/// @brief receives a fen string and a pointer to bitboards array. init state
/// @param fen_string 
/// @param bitboards 
void setup_state_from_fen(board_state* state, const char* fen_string){ 
    state->data = state->metadata; // data pointer starts by pointing to first element in metadata array

    char* end;
    char* copy = strdup(fen_string);

    char* t = strtok(copy, " ");

    setup_bitboards(state, t);

    state->data->s = (*strtok(NULL, " ") == 'w') ? WHITE : BLACK;

    char* castling_rights = strtok(NULL, " ");
    char c;

    state->data->castling_rights = 0;

    while((c = *castling_rights++)){
        switch(c){
            case 'K': state->data->castling_rights |= K_castle; break;
            case 'Q': state->data->castling_rights |= Q_castle; break;
            case 'k': state->data->castling_rights |= k_castle; break;
            case 'q': state->data->castling_rights |= q_castle; break;
        }
    }

    state->data->ep_square = char_to_square(strtok(NULL, " "));
    state->data->move = 0;

    if(state->data->ep_square != s_none){
        // make previous move a double pawn push that would've led to this en-passant square
        
        castling_and_enpassant_info cep = cep_info[state->data->s];

        state->data->move = (1 << 12) | ((state->data->ep_square - cep.ep_sq_offset) << 6) | (state->data->ep_square + cep.ep_sq_offset);
    }


    state->data->hisply = strtol(strtok(NULL, " "), &end, 10);
    state->data->moves = strtol(strtok(NULL, " "), &end, 10);
    state->data->captured_piece = p_none;
}

/// @brief Populate bitboards from fen
/// @param board_string 
void setup_bitboards(board_state* state, const char* fen){
    int pointer = 0, current_square = 63;
    char c;
    piece piece = p_none;
    
    memset(state->bitboards, 0, sizeof(state->bitboards));

    for(int i = 0; i < 64; ++i){state->board[i] = p_none;}
    state->data->occupied = 0ULL;

    while(current_square >= 0){
        c = fen[pointer];

        if(isalpha(c)){
            piece = char_to_piece(c);
        
            if(piece != p_none){
                state->bitboards[piece] |= set_bit(current_square);
                state->board[current_square] = piece;
                state->data->occupied |= set_bit(current_square);
            }

            current_square--;

        } else if(isdigit(c)){
            current_square -= (c - '0');
        }

        pointer++;
    }
}

void print_board(const board_state* state){
    piece p;
    char c; 

    printf("castling rights flag: %d\n", state->data->castling_rights);
    printf("total ply: %d\n", state->data->hisply);
    printf("moves: %d\n", state->data->moves);
    printf("previous move: ");
    print_move(state->data->move);
    printf("turn: %s\n", (state->data->s) ? "b" : "w");
    printf("White eval: %d Black eval: %d \n", state->data->eval[WHITE], state->data->eval[BLACK]);

    printf("----------------\n");
    for(int i = 63; i >= 0; i--){
        p = state->board[i];
        
        c = (p == p_none) ? '.' : char_pieces[p];
    
        printf("%c ", c);  

        if((i % 8) == 0){
            printf("|%d\n", (i/8) + 1);
        }
    }
    printf("----------------\n");
    printf("a b c d e f g h\n\n");

    printf("Key: %lx \n", state->data->hash);
}

