#include "evaluation.h"

Eval::Eval(){}

Eval::Eval(Board* _board) : board(_board) {}

int Eval::count_white_material(){
    int material = 0;

    std::vector<piece_names> pieces = {N, P, Q, R, B};

    for(piece_names piece: pieces){
        material += count_set_bits(board->get_piece_bitboard(piece)) * get_piece_value[piece];                                      
    }

    return material;
}

int Eval::count_black_material(){
    int material = 0;

    std::vector<piece_names> pieces = {n, p, q, r, b};

    for(piece_names piece: pieces){
        material += count_set_bits(board->get_piece_bitboard(piece)) * get_piece_value[piece-8];
    }
    
    return material;
}


int Eval::Evaluation(){
    auto perspective = board->get_turn() ? -1 : 1;
    
    return perspective * (count_white_material() - count_black_material());
}

 






