#include "engine.h"

/// Minimax with no optimisations
int Enginev0::plain_minimax(int depth){
    if(depth == 0){
        return eval.Evaluation();
    }

    std::vector<Move> moves = movegen->generate_moves(); 

    if(moves.size() == 0){
        if(movegen->ally_king_in_check()){
            return -infinity;  // checkmate
        } else {
            return 0;         // stalemate
        }
    }

    int curr_eval = 0, best_eval = -infinity;

    for(Move& move : moves){
        make_move(move);
        curr_eval = -plain_minimax(depth-1);
        best_eval = std::max(curr_eval, best_eval);
        board->undo_move();
    }

    return best_eval;
}  

Move Enginev0::get_engine_move(){
    int best_eval = -infinity, curr_eval;
    Move best_move;
    int perspective = board->get_turn() ? -1 : 1;

    std::vector<Move> moves = movegen->generate_moves(); 

    eval.nodes_searched = 0;

    for(Move& move : moves){
        make_move(move);

        curr_eval = perspective * plain_minimax(depth-1);

        if(curr_eval > best_eval){
            best_eval = curr_eval;
            best_move = move;
        }   

        board->undo_move();
    }

    return best_move;
}

/// Minimax with alpha beta
int Enginev1::alpha_beta_minimax(int depth, int alpha, int beta){
    if(depth == 0){
        return eval.Evaluation();
    }

    std::vector<Move> moves = movegen->generate_moves(); 

    if(moves.size() == 0){
        if(movegen->ally_king_in_check()){
            return -infinity;  // checkmate
        } else {
            return 0;         // stalemate
        }
    } 

    int curr_eval = 0;

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

Move Enginev1::get_engine_move(){
    int best_eval = -infinity, curr_eval;
    Move best_move;
    int perspective = board->get_turn() ? -1 : 1;

    std::vector<Move> moves = movegen->generate_moves(); 

    eval.nodes_searched = 0;

    for(Move& move : moves){
        make_move(move);

        curr_eval = perspective * alpha_beta_minimax(depth-1,-infinity, infinity);

        if(curr_eval > best_eval){
            best_eval = curr_eval;
            best_move = move;
        }

        board->undo_move();
    }

    return best_move;
}

/// Given a set of moves, use hueristics to guess its quality. Used for move ordering
void Enginev2::set_move_heuristics(std::vector<Move>& moves){
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
        // bitboard of enemy pawns
        U64 enemy_pawns = board->get_turn() ? board->get_piece_bitboard(P) : board->get_piece_bitboard(p); 

        move_score -= PAWN_ATTACK_POWER * movegen->get_attackers(move.get_to(), ~board->get_turn()) & enemy_pawns;

        move.value = move_score;
    }
}

void Enginev2::order_moves(std::vector<Move>& moves){      
    set_move_heuristics(moves);
    std::sort(moves.begin(), moves.end());
}

int Enginev2::ab_move_ordering(int depth, int alpha, int beta){
    if(depth == 0){
        return quiescence(alpha, beta);
    }

    std::vector<Move> moves = movegen->generate_moves(); 

    if(moves.size() == 0){
        if(movegen->ally_king_in_check()){
            return -infinity;  // checkmate
        } else {
            return 0;         // stalemate
        }
    } 

    int curr_eval = 0;

    order_moves(moves);

    for(Move& move : moves){
        make_move(move);
        curr_eval = -ab_move_ordering(depth-1, -beta, -alpha);
        
        alpha = std::max(curr_eval, alpha);
        board->undo_move();

        if(curr_eval >= beta){
            return beta;
        }
    }

    return alpha;
}

Move Enginev2::get_engine_move(){
    int best_eval = -infinity, curr_eval;
    Move best_move;
    int perspective = board->get_turn() ? -1 : 1;

    std::vector<Move> moves = movegen->get_legal_moves(); 

    eval.nodes_searched = 0;

    order_moves(moves);

    for(Move& move : moves){
        make_move(move);

        curr_eval = perspective * ab_move_ordering(depth-1,-infinity, infinity);

        if(curr_eval > best_eval){
            best_eval = curr_eval;
            best_move = move;
        }

        board->undo_move();
    }

    return best_move;
}

int Enginev2::quiescence(int alpha, int beta){
    int evaluation = eval.Evaluation();

    if(evaluation >= beta){
        return beta;
    }

    alpha = std::max(evaluation, alpha);
    
    // consider only capture moves
    std::vector<Move> moves = movegen->generate_moves(true); 

    int curr_eval = 0;

    order_moves(moves);

    for(Move& move : moves){
        make_move(move);
        curr_eval = -quiescence(-beta, -alpha);
        board->undo_move();

        alpha = std::max(curr_eval, alpha);

        if(curr_eval >= beta){
            return beta;
        }
    }

    return alpha;
}

void Engine::engine_driver(){
    board->view_board();   

    auto start = high_resolution_clock::now();

    Move move = get_engine_move();

    auto end = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>(end-start);

    std::cout << "Nodes searched: " << eval.nodes_searched << std::endl;
    std::cout << "Time taken: " << std::to_string(duration.count()) << " ms" << std::endl;
    std::cout << move << std::endl;

    board->make_move(move);
    movegen->generate_moves();

}


