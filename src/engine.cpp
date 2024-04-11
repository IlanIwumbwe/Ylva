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

Move Enginev0::get_engine_move(std::vector<Move>& moves){
    int best_eval = -infinity, curr_eval;
    Move best_move = moves[0];

    eval.nodes_searched = 0;

    for(Move& move : moves){
        make_move(move);

        curr_eval = -plain_minimax(depth-1);

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

Move Enginev1::get_engine_move(std::vector<Move>& moves){
    int best_eval = -infinity, curr_eval;
    Move best_move = moves[0];

    eval.nodes_searched = 0;

    for(Move& move : moves){
        make_move(move);

        curr_eval = -alpha_beta_minimax(depth-1,-infinity, infinity);

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
    int move_value, from_val, to_val;

    piece_names from_piece, to_piece;

    for(Move& move : moves){
        move_value = 0;

        from_piece = board->get_piece_on_square(move.get_from());
        from_val = get_piece_value[from_piece];
        
        // capturing high value piece with low value piece is good
        if(move.is_capture()){
            to_piece = board->get_piece_on_square(move.get_to());
            to_val = get_piece_value[to_piece];

            move_value += CAPTURE_VAL_POWER * std::max(0, to_val - from_val);
        }
        
        // promotion is good
        if(move.is_promo()){
            move_value += PROMOTION_POWER;
        }

        // moving into square attacked by enemy pawn is bad   
        U64 pawn_attackers = 0;

        movegen->get_pawn_attackers(pawn_attackers, move.get_to(), ~board->get_turn());

        move_value -= PAWN_ATTACK_POWER * count_set_bits(pawn_attackers);

        move.value = move_value;
    }
}

void Enginev2::order_moves(std::vector<Move>& moves){      
    set_move_heuristics(moves);
    std::sort(moves.begin(), moves.end());
}

int Enginev2::ab_move_ordering(int depth, int alpha, int beta){
    if(depth == 0){
        //return eval.Evaluation(); 
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

Move Enginev2::get_engine_move(std::vector<Move>& moves){
    int best_eval = -infinity, curr_eval;
    Move best_move;

    eval.nodes_searched = 0;

    order_moves(moves);

    best_move = moves[0];

    for(Move& move : moves){
        make_move(move);
        curr_eval = -ab_move_ordering(depth-1,-infinity, infinity);

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
    std::vector<Move> capture_moves = movegen->generate_moves(true); 

    int curr_eval = 0;

    order_moves(capture_moves);

    for(Move& move : capture_moves){
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

    std::vector<Move> legal_moves = movegen->get_legal_moves();

    auto start = high_resolution_clock::now();

    Move move = get_engine_move(legal_moves);

    auto end = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>(end-start);

    std::cout << "Nodes searched: " << eval.nodes_searched << std::endl;
    std::cout << "Time taken: " << std::to_string(duration.count()) << " ms" << std::endl;
    std::cout << move << std::endl;

    board->make_move(move);
    movegen->generate_moves();
}


