#include "evaluation.h"

Eval::Eval(Board* _board, MoveGen* _movegen) : board(_board), movegen(_movegen) {}

int Eval::count_white_material(){
    int material = 0;

    material += count_set_bits(board->get_piece_bitboard(N)) * KNIGHT;
    material += count_set_bits(board->get_piece_bitboard(P)) * PAWN;
    material += count_set_bits(board->get_piece_bitboard(Q)) * QUEEN;
    material += count_set_bits(board->get_piece_bitboard(R)) * ROOK;
    material += count_set_bits(board->get_piece_bitboard(B)) * BISHOP;

    return material;
}

int Eval::count_black_material(){
    int material = 0;

    material += count_set_bits(board->get_piece_bitboard(p)) * PAWN;
    material += count_set_bits(board->get_piece_bitboard(n)) * KNIGHT;
    material += count_set_bits(board->get_piece_bitboard(q)) * QUEEN;
    material += count_set_bits(board->get_piece_bitboard(r)) * ROOK;
    material += count_set_bits(board->get_piece_bitboard(b)) * BISHOP;

    return material;
}

int Eval::Evaluation(){
    auto perspective = board->get_turn() ? -1 : 1;
    
    return perspective * (count_white_material() - count_black_material());
}

/// Minimax with no optimisations
float Eval::PlainMinimax(int depth){
    if(depth == 0){
        return Evaluation();
    }

    auto moves = board->get_valid_moves();
    if(moves.size() == 0){
        if(movegen->ally_king_in_check()){
            return -INFINITY;  // checkmate
        } else {
            return 0.0;         // stalemate
        }
    }

    float curr_eval = 0.0, best_eval = -INFINITY;

    for(auto move : moves){
        make_move(move);
        curr_eval = -PlainMinimax(depth-1);
        best_eval = std::max(curr_eval, best_eval);
        board->undo_move();
    }

    return best_eval;
}



