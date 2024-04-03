#include "engine.h"

Engine::Engine(){}

Engine::Engine(Board* board, MoveGen* movegen, int depth) : board(board), movegen(movegen), eval(board, movegen), depth(depth){
    std::cout << "Searching to depth " << depth << std::endl;
}

int Engine::get_random_index(int moves_size){
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> dist(0, moves_size-1);

    return dist(gen);
}

Move Engine::find_minimax_move(){
    float best_eval = -INFINITY, curr_eval;
    Move best_move;
    int perspective = board->get_turn() ? -1 : 1;

    std::vector<Move> moves = board->get_valid_moves();

    eval.nodes_searched = 0;

    for(Move& move : moves){
        make_move(move);

        curr_eval = perspective * eval.plain_minimax(depth-1);

        if(curr_eval > best_eval){
            best_eval = curr_eval;
            best_move = move;
        }

        board->undo_move();
    }

    return best_move;
}

Move Engine::find_alphabeta_move(){
    float best_eval = -INFINITY, curr_eval;
    Move best_move;
    int perspective = board->get_turn() ? -1 : 1;

    std::vector<Move> moves = board->get_valid_moves();

    eval.nodes_searched = 0;

    eval.order_moves(moves);

    for(Move& move : moves){
        make_move(move);

        curr_eval = perspective * eval.alpha_beta_minimax(depth-1,-INFINITY, INFINITY);

        if(curr_eval > best_eval){
            best_eval = curr_eval;
            best_move = move;
        }

        board->undo_move();
    }

    return best_move;
}

Move Engine::get_random_move(){
    auto moves = board->get_valid_moves();
    return moves[get_random_index(moves.size())];
}

void Engine::make_engine_move(){
    board->view_board();   

    auto start = high_resolution_clock::now();

    auto move = find_alphabeta_move(); // find_minimax_move(); //get_random_move();

    auto end = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>(end-start);

    std::cout << "Nodes searched: " << eval.nodes_searched << std::endl;
    std::cout << "Time taken: " << std::to_string(duration.count()) << " ms" << std::endl;
    std::cout << move << std::endl;

    make_move(move);
}   

