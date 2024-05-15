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
const int* PSQT[6] = {PAWN, KING, QUEEN, ROOK, KNIGHT, BISHOP};

/// Swap the move at this start index with the move that has the highest score
void Enginev2::pick_move(std::vector<Move>& moves, int start_index){

    for(size_t i = start_index+1; i < moves.size(); ++i){
        if(moves[i].value > moves[start_index].value){
            // swap
            Move tmp = moves[start_index];
            moves[start_index] = moves[i];
            moves[i] = tmp;
        }
    }
}

/// @brief Check that the move was legal in the original position
/// @param legal_moves 
/// @param move 
bool move_exists(std::vector<Move>& legal_moves, Move move){
    for(Move v_move : legal_moves){
        if(v_move == move){
            return true;
        }
    }

    return false;
}

/// @brief Get principle variation from pv table
/// @param depth 
/// @param position 
int Engine::get_pv_line(int depth, std::vector<Move>& initial_legal_moves){
    Move move = Move(probe_pv_table(board));
    int count = 0;
    int initial_ply = board->ply;
    std::vector<Move> legal_moves = initial_legal_moves;


    // std::cout << "Pos before pv line: " << std::hex << board->hash_key << std::endl; 

    while(!move.is_no_move() && (count < depth)){

        assert(count < MAX_DEPTH);

        if(move_exists(legal_moves, move)){
            board->make_move(move);
            board->pv_array[count++] = move.get_move();
            legal_moves = movegen->generate_moves();
        } else {
            break;
        }

        move = probe_pv_table(board);
    }

    // std::cout << "Pos after checking for pv line: " << std::hex << board->hash_key << std::endl; 

    while(board->ply != initial_ply){
        board->undo_move();
    }

    // std::cout << "Pos after getting pv line: " << std::hex << board->hash_key << std::endl; 

    return count;
}

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
        board->undo_move();

        if(curr_eval > best_eval){
            best_eval = curr_eval;
            store_pv_move(board, move.get_move());
        }
    }

    return best_eval;
}  

void Enginev0::get_engine_move(std::vector<Move>& legal_moves){
    int best_eval = -infinity, curr_eval;

    for(Move& move : legal_moves){
        make_move(move);

        curr_eval = -plain_minimax(depth-1);

        board->undo_move();

        if(curr_eval > best_eval){
            best_eval = curr_eval;
            store_pv_move(board, move.get_move());
        }   
    }
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
        board->undo_move();

        if(curr_eval > alpha){
            alpha = curr_eval;
            store_pv_move(board, move.get_move());
        }

        if(curr_eval >= beta){
            return beta;
        }
    }

    return alpha;
}

void Enginev1::get_engine_move(std::vector<Move>& legal_moves){
    int best_eval = -infinity, curr_eval;

    for(Move& move : legal_moves){
        make_move(move);

        curr_eval = -alpha_beta_minimax(depth-1,-infinity, infinity);

        board->undo_move();

        if(curr_eval > best_eval){
            best_eval = curr_eval;
            store_pv_move(board, move.get_move());
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

    for(size_t i = 0; i < moves.size(); ++i){
        pick_move(moves, i);
        make_move(moves[i]);

        curr_eval = -ab_move_ordering(depth-1, -beta, -alpha);

        board->undo_move();

        if(curr_eval > alpha){
            alpha = curr_eval;
            store_pv_move(board, moves[i].get_move());
        }

        if(curr_eval >= beta){
            return beta;
        }
    }

    return alpha;
}

/// Given a set of moves, use hueristics to guess its quality. Used for move ordering
void Enginev2::set_move_heuristics(std::vector<Move>& moves){
    piece_names from_piece, to_piece;
    uint move_flag;

    for(Move& move : moves){
        from_piece = board->get_piece_on_square(move.get_from());
        to_piece = board->get_piece_on_square(move.get_to());
        move_flag = move.get_flags();

        int from_piece_as_index = convert_piece_to_index(from_piece);
        int to_piece_as_index = convert_piece_to_index(to_piece);

        // use mvv_lva to sort capture moves by how much material they will gain 
        move.value += MVV_LVA[to_piece_as_index][from_piece_as_index];

        // promotion is good
        move.value += PROMOTION_POWER * (int)move.is_promo() * ((move_flag & 0x7) + 1);

        // moving into square attacked by enemy pawn is bad   
        U64 pawn_attackers = movegen->get_pawn_attackers(move.get_to(), ~board->get_turn());

        move.value -= (PAWN_ATTACK_POWER * count_set_bits(pawn_attackers));
    }
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

    for(size_t i = 0; i < capture_moves.size(); ++i){
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

void Enginev2::search_position(std::vector<Move>& moves, int search_depth){
    int best_eval = -infinity, curr_eval;

    set_move_heuristics(moves);

    for(size_t i = 0; i < moves.size(); ++i){
        pick_move(moves, i);   
        make_move(moves[i]);

        curr_eval = -ab_move_ordering(search_depth-1,-infinity, infinity);

        board->undo_move();

        if(curr_eval > best_eval){
            best_eval = curr_eval;
            store_pv_move(board, moves[i].get_move());
        }
    }
}

void Enginev2::get_engine_move(std::vector<Move>& legal_moves){

    // one shot search
    search_position(legal_moves, depth);
}


void Engine::engine_driver(){
    board->view_board();   
    std::vector<Move> legal_moves = movegen->get_legal_moves();

    auto start = high_resolution_clock::now();

    get_engine_move(legal_moves);
    int pv_lenth = get_pv_line(depth, legal_moves);

    if(legal_moves.size() == 0 || pv_lenth == 0){
        std::cout << "pv len: " << pv_lenth << ", leg: " << legal_moves.size() << std::endl;
    }
    
    assert(pv_lenth > 0);

    // best move is top of pv array
    Move best_move(board->pv_array[0]);

    auto end = high_resolution_clock::now();

    milliseconds duration = duration_cast<milliseconds>(end-start);

    std::cout << "Nodes searched: " << std::dec << eval.nodes_searched << std::endl;
    std::cout << "Time taken: " << std::to_string(duration.count()) << " ms" << std::endl;
    
    std::cout << "Principal variation " << std::endl;

    for(int i = 0; i < pv_lenth; ++i){
        std::cout << Move(board->pv_array[i]) << " ";
    }

    std::cout << "\n";

    time_used_per_turn = duration_cast<seconds>(end-start);

    eval.nodes_searched = 0;

    board->make_move(best_move);
    movegen->generate_moves();
}



