#include "engine.h"

Engine::Engine(Board* board, MoveGen* movegen) : board(board), movegen(movegen){

}

int Engine::get_random_index(int moves_size){
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> dist(0, moves_size-1);

    return dist(gen);
}

Move Engine::get_best_move(){
}

Move Engine::get_random_move(){
    auto moves = board->get_valid_moves();
    return moves[get_random_index(moves.size())];
}

void Engine::make_engine_move(){
    board->view_board();   

    auto move = get_random_move();
    std::cout << move << std::endl;

    board->make_move(move);    
    movegen->generate_moves(); 
}   

