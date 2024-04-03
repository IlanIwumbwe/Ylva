#include "evaluation.h"

Eval::Eval(){}

Eval::Eval(Board* _board, MoveGen* _movegen) : board(_board), movegen(_movegen) {}

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
        material += count_set_bits(board->get_piece_bitboard(piece)) * get_piece_value[piece];
    }
    
    return material;
}

/// Given a set of moves, use hueristics to guess its quality. Used for move ordering
void Eval::set_move_heuristics(std::vector<Move>& moves, U64& enemy_pawns){
    int move_score, from_val, to_val;

    piece_names from_piece, to_piece;

    for(Move& move : moves){
        move_score = 0;

        from_piece = board->get_piece_on_square(move.get_from());
        from_val = get_piece_value[from_piece];
        
        // capturing high value piece with low value piece is good
        if(move.get_move() & CAPTURE_FLAG){
            to_piece = board->get_piece_on_square(move.get_to());
            to_val = get_piece_value[to_piece];

            move_score += CAPTURE_VAL_POWER * std::max(0, to_val - from_val);
        }
        
        // promotion is good
        move_score += (move.get_move() & PROMO_FLAG) * (PROMOTION_POWER / 32768);

        // moving into square attacked by enemy pawn is bad    
        move_score -= std::min((set_bit(move.get_to()) & enemy_pawns) << 6, PAWN_ATTACK_POWER);

        move.value = move_score;
    }
}

void Eval::order_moves(std::vector<Move>& moves){      
    /// bitboard of enemy pawns
    U64 enemy_pawns = board->get_turn() ? board->get_piece_bitboard(P) : board->get_piece_bitboard(p);

    set_move_heuristics(moves, enemy_pawns);
    std::sort(moves.begin(), moves.end());
}

int Eval::Evaluation(){
    auto perspective = board->get_turn() ? -1 : 1;
    
    return perspective * (count_white_material() - count_black_material());
}

/// Minimax with no optimisations
float Eval::plain_minimax(int depth){
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
        curr_eval = -plain_minimax(depth-1);
        best_eval = std::max(curr_eval, best_eval);
        board->undo_move();
    }

    return best_eval;
}   

/// Minimax with alpha beta
float Eval::alpha_beta_minimax(int depth, float alpha, float beta){
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

    order_moves(moves);

    float curr_eval = 0.0;

    for(Move& move : moves){
        make_move(move);
        curr_eval = -alpha_beta_minimax(depth-1, -beta, -alpha);
        
        alpha = std::max(curr_eval, alpha);
        board->undo_move();

        if(curr_eval >= beta){
            return beta;
        }
    }

    return alpha;
}




