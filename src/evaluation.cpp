#include "evaluation.h"

Eval::Eval(){}

Eval::Eval(Board* _board, MoveGen* _movegen) : board(_board), movegen(_movegen) {}

int Eval::count_white_material(){
    int material = 0;

    material += count_set_bits(board->get_piece_bitboard(N)) * KNIGHT_VAL;
    material += count_set_bits(board->get_piece_bitboard(P)) * PAWN_VAL;
    material += count_set_bits(board->get_piece_bitboard(Q)) * QUEEN_VAL;
    material += count_set_bits(board->get_piece_bitboard(R)) * ROOK_VAL;
    material += count_set_bits(board->get_piece_bitboard(B)) * BISHOP_VAL;

    return material;
}

int Eval::count_black_material(){
    int material = 0;

    material += count_set_bits(board->get_piece_bitboard(p)) * PAWN_VAL;
    material += count_set_bits(board->get_piece_bitboard(n)) * KNIGHT_VAL;
    material += count_set_bits(board->get_piece_bitboard(q)) * QUEEN_VAL;
    material += count_set_bits(board->get_piece_bitboard(r)) * ROOK_VAL;
    material += count_set_bits(board->get_piece_bitboard(b)) * BISHOP_VAL;

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

    std::vector<Move> moves = board->get_valid_moves();

    if(moves.size() == 0){
        if(movegen->ally_king_in_check()){
            return -INFINITY;  // checkmate
        } else {
            return 0.0;         // stalemate
        }
    }

    float curr_eval = 0.0, best_eval = -INFINITY;

    for(Move& move : moves){
        make_move(move);
        curr_eval = -PlainMinimax(depth-1);
        best_eval = std::max(curr_eval, best_eval);
        board->undo_move();
    }

    return best_eval;
}   
/// Minimax with alpha beta
float Eval::AlphaBetaMinimax(int depth, float alpha, float beta){
    if(depth == 0){
        return Evaluation();
    }

    std::vector<Move> moves = board->get_valid_moves();

    if(moves.size() == 0){
        if(movegen->ally_king_in_check()){
            return -INFINITY;  // checkmate
        } else {
            return 0.0;         // stalemate
        }
    }   

    float curr_eval = 0.0;

    for(Move& move : moves){
        make_move(move);
        curr_eval = -AlphaBetaMinimax(depth-1, -beta, -alpha);
        
        alpha = std::max(curr_eval, alpha);
        board->undo_move();

        if(curr_eval >= beta){
            return beta;
        }
    }

    return alpha;
}




