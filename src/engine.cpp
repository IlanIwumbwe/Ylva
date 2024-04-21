#include "engine.h"

// most valuable victim, least valuable attacker heuristic

/*
    The values for this table are got from https://rustic-chess.org/search/ordering/mvv_lva.html

    attacker none should not be possible
    victim none having a score of 0 here means we don't have to check whether a move is a capture move
    moves that attack the king can't capture the king so therefore gain no bonus

    piece indexing from enum definition:
    None = 0,
    P = 1 , 
    K = 2 ,
    Q = 3 , 
    R = 4 ,
    N = 5 ,
    B = 6 ,
*/
const int MVV_LVA[7][7] = {
    {0, 0, 0, 0, 0, 0, 0},          // victim = None, attacker = None, Pawn, King, Queen, Rook, Knight, Bishop
    {0, 15, 10, 11, 12, 14, 13},    // victim = Pawn, attacker = None, Pawn, King, Queen, Rook, Knight, Bishop
    {0, 0, 0, 0, 0, 0, 0},          // victim = King, attacker = None, Pawn, King, Queen, Rook, Knight, Bishop
    {0, 55, 50, 51, 52, 54, 53},    // victim = Queen, attacker = None, Pawn, King, Queen, Rook, Knight, Bishop
    {0, 45, 40, 41, 42, 44, 43},    // victim = Rook, attacker = None, Pawn, King, Queen, Rook, Knight, Bishop},
    {0, 25, 20, 21, 22, 24, 23},    // victim = Knight, attacker = None, Pawn, King, Queen, Rook, Knight, Bishop
    {0, 35, 30, 31, 32, 34, 33}     // victim = Bishop, attacker = None, Pawn, King, Queen, Rook, Knight, Bishop
};

// piece square tables

/*
    These have been copied from: https://www.chessprogramming.org/Simplified_Evaluation_Function
    The point of these is to score positions which promote pieces into favourable squares more highly than positions that don't

    Because of my convention, to index this boards by square, you have to do 63 - square 
*/

const int PAWN[32] = {
    0,  0,  0,  0, 
    50, 50, 50, 50,
    10, 10, 20, 30,
    5,  5, 10, 25, 
    0,  0,  0, 20, 
    5, -5,-10,  0, 
    5, 10, 10,-20, 
    0,  0,  0,  0
};

const int KNIGHT[32] = {
    -50,-40,-30,-30,
    -40,-20,  0,  0,
    -30,  0, 10, 15,
    -30,  5, 15, 20,
    -30,  0, 15, 20,
    -30,  5, 10, 15,
    -40,-20,  0,  5,
    -50,-40,-30,-30
};

const int BISHOP[32] = {
    -20,-10,-10,-10,
    -10,  0,  0,  0,
    -10,  0,  5, 10,
    -10,  5,  5, 10,
    -10,  0, 10, 10,
    -10, 10, 10, 10,
    -10,  5,  0,  0,
    -20,-10,-10,-10
};

const int ROOK[32] = {
    0,  0,  0,  0, 
    5, 10, 10, 10, 
    -5,  0,  0,  0,
    -5,  0,  0,  0,
    -5,  0,  0,  0,
    -5,  0,  0,  0,
    -5,  0,  0,  0,
    0,  0,  0,  5
};

const int QUEEN[32] = {
    -20,-10,-10, -5,
    -10,  0,  0,  0,
    -10,  0,  5,  5,
    -5,  0,  5,  5, 
     0,  0,  5,  5, 
    -10,  5,  5,  5,
    -10,  0,  5,  0,
    -20,-10,-10, -5 
};

// middlegame scores only
/// TODO: think about how to merge both middlegame and endgame tables nicely
const int KING[32] = {
    30,-40,-40,-50,
    -30,-40,-40,-50,
    -30,-40,-40,-50,
    -30,-40,-40,-50,
    -20,-30,-30,-40,
    -10,-20,-20,-20,
    20, 20,  0,  0, 
    20, 30, 10,  0
};

/// To index these tables, make sure to subtract one from the piece index
const int* PIECE_SQUARE_TABLES[6] = {PAWN, KING, QUEEN, ROOK, KNIGHT, BISHOP};

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
    std::vector<Move> legal_moves = movegen->get_legal_moves();
    Move best_move = legal_moves[0];

    for(Move& move : legal_moves){
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

Move Enginev1::get_engine_move(){
    int best_eval = -infinity, curr_eval;
    std::vector<Move> legal_moves = movegen->get_legal_moves();
    Move best_move = legal_moves[0];

    for(Move& move : legal_moves){
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
    piece_names from_piece, to_piece;

    for(Move& move : moves){
        from_piece = board->get_piece_on_square(move.get_from());
        to_piece = board->get_piece_on_square(move.get_to());

        int from_piece_as_index = map_piece_index(from_piece);
        int to_piece_as_index = map_piece_index(to_piece);

        // use mvv_lva to sort capture moves by how much material they will gain 
        move.value += MVV_LVA[to_piece_as_index][from_piece_as_index];

        // move.value += (CAPTURE_VAL_POWER * std::max(0, get_piece_value[to_ind] - get_piece_value[from_ind]));

        if(!move.is_capture()){
            int to_square_as_index = map_square_index(move.get_to());
            move.value += PIECE_SQUARE_TABLES[from_piece_as_index-1][to_square_as_index];
        }

        // promotion is good
        move.value += (PROMOTION_POWER & (int)(move.is_promo()));

        // moving into square attacked by enemy pawn is bad   
        U64 pawn_attackers = movegen->get_pawn_attackers(move.get_to(), ~board->get_turn());

        move.value -= (PAWN_ATTACK_POWER * count_set_bits(pawn_attackers));
    }
}

/// Swap the move at this start index with the move that has the highest score
void Enginev2::pick_move(std::vector<Move>& moves, int start_index){

    for(int i = start_index+1; i < (int)moves.size(); ++i){
        if(moves[i].value > moves[start_index].value){
            // swap
            Move tmp = moves[start_index];
            moves[start_index] = moves[i];
            moves[i] = tmp;
        }
    }
}

int Enginev2::ab_move_ordering(int depth, int alpha, int beta){
    if(depth == 0){
        // return eval.Evaluation(); 
        return quiescence(alpha, beta);
    }

    std::vector<Move> moves = movegen->generate_moves(); 

    if(movegen->no_legal_moves()){
        if(movegen->ally_king_in_check()){
            return -infinity;  // checkmate
        } else {
            return 0;         // stalemate
        }
    } 

    int curr_eval = 0;

    set_move_heuristics(moves);

    for(int i = 0; i < (int)moves.size(); ++i){
        pick_move(moves, i);

        make_move(moves[i]);
        curr_eval = -ab_move_ordering(depth-1, -beta, -alpha);
        alpha = std::max(curr_eval, alpha);
        board->undo_move();

        if(curr_eval >= beta){
            return beta;
        }
    }

    return alpha;
}
 
/*
    Perform a new search that looks only at capture moves
*/
int Enginev2::quiescence(int alpha, int beta){
    int evaluation = eval.Evaluation();

    if(evaluation >= beta){
        return beta;
    }

    alpha = std::max(evaluation, alpha);
    
    // consider only capture moves
    std::vector<Move> capture_moves = movegen->generate_moves(true); 

    int curr_eval = 0;
    
    set_move_heuristics(capture_moves);

    for(int i = 0; i < (int)capture_moves.size(); ++i){
        pick_move(capture_moves, i);

        make_move(capture_moves[i]);
        curr_eval = -quiescence(-beta, -alpha);
        board->undo_move();

        alpha = std::max(curr_eval, alpha);

        if(curr_eval >= beta){
            return beta;
        }
    }

    return alpha;
}

Move Enginev2::search_position(std::vector<Move>& moves, int search_depth){
    int best_eval = -infinity, curr_eval;
    Move best_move = moves[0];

    set_move_heuristics(moves);

    for(int i = 0; i < (int)moves.size()-1; ++i){
        pick_move(moves, i);

        make_move(moves[i]);

        curr_eval = -ab_move_ordering(search_depth-1,-infinity, infinity);

        moves[i].value += curr_eval; 

        if(curr_eval > best_eval){
            best_eval = curr_eval;
            best_move = moves[i];
        }

        board->undo_move();

    }

    return best_move; 
}

Move Enginev2::get_engine_move(){
    Move best_move;
    std::vector<Move> legal_moves = movegen->get_legal_moves();

    // iterative deepening
    /*
    for(int current_depth = 1; current_depth <= depth; ++current_depth){
        eval.nodes_searched = 0;
        best_move = search_position(legal_moves, current_depth);
    }
    */
    
    // one shot search
    best_move = search_position(legal_moves, depth);
    
    return best_move;
}


void Engine::engine_driver(){
    board->view_board();   

    auto start = high_resolution_clock::now();

    Move move = get_engine_move();

    auto end = high_resolution_clock::now();

    milliseconds duration = duration_cast<milliseconds>(end-start);

    std::cout << "Nodes searched: " << eval.nodes_searched << std::endl;
    std::cout << "Time taken: " << std::to_string(duration.count()) << " ms" << std::endl;
    std::cout << move << std::endl;

    time_used_per_turn = duration_cast<seconds>(end-start);

    board->make_move(move);
    movegen->generate_moves();
}


