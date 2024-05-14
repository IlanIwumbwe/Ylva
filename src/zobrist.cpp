#include "zobrist.h"

U64 Piece_keys[13][64];
U64 Turn_key;
U64 Castle_key[16];

void init_hash_keys(){
    for(int i = 0; i < 13; ++i){
        for(int j = 0; j < 64; ++j){
            Piece_keys[i][j] = RAND64;
        }
    }

    Turn_key = RAND64;

    for(int i = 0; i < 16; ++i){
        Castle_key[i] = RAND64;
    }
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

    if(ep_square){
        final_key ^= Piece_keys[None][ep_square];
    }

    final_key ^= Castle_key[position->get_castling_rights()];

    position->hash_key = final_key;
}
