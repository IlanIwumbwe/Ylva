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

    auto moves = board->get_valid_moves();

    std::cout << "valids" << std::endl;
    for (Move move : moves){
        std::cout << move << std::endl;
    }

    for(auto move : moves){
        make_move(move);
        curr_eval = perspective * eval.AlphaBetaMinimax(depth-1,-INFINITY, INFINITY);
        //curr_eval = perspective * eval.PlainMinimax(depth-1);

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

    auto move = find_minimax_move(); //get_random_move();
    std::cout << move << std::endl;

    make_move(move);
}   

