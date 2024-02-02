#include "engine.h"

Engine::Engine(Board* board, MoveGen* movegen) : board(board), movegen(movegen), eval(board, movegen){

}

int Engine::get_random_index(int moves_size){
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> dist(0, moves_size-1);

    return dist(gen);
}

Move Engine::find_minimax_move(){
    int best_eval = -INFINITY, curr_eval;
    Move best_move;

    auto moves = board->get_valid_moves();

    for(auto move : moves){
        make_move(move);
        curr_eval = eval.PlainMinimax(depth - 1) + eval.Evaluation();

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

