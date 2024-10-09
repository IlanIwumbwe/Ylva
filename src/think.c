#include "../headers/think.h"

int eval(){
    int perspective = (board_info->s == WHITE) ? 1 : -1;
    return perspective * (board_info->material[WHITE] - board_info->material[BLACK]);   
}

int search(int depth, int alpha, int beta){

    if(depth == 0){
        return eval(); 
    }

    U16 move;
    int eval;

    dynamic_array moves_array;
    init_da(&moves_array, 218);

    generate_moves(&moves_array, 0);

    if(moves_array.used == 0){
        if(n_checkers){
            return -INT_MAX; // checkmate
        } else {
            return 0;  // stalemate
        }
        
    }

    for(size_t i = 0; i < moves_array.used; ++i){
        move = moves_array.array[i];

        make_move(move);

        eval = -search(depth - 1, -beta, -alpha);

        undo_move();

        if(eval > alpha){
            alpha = eval;
        }

        if(eval >= beta){
            return beta; // beta cutoff
        }

    }

    free_da(&moves_array);
    
    return alpha;

}

void think(int depth){

    dynamic_array moves_array;
    init_da(&moves_array, 218);

    generate_moves(&moves_array, 0);

    U16 curr_move;
    U16 best_move = 0;
    int best_eval = -INT_MAX;
    int eval = 0;

    for(size_t i = 0; i < moves_array.used; ++i){
        curr_move = moves_array.array[i];

        make_move(curr_move);

        eval = -search(depth-1, -INT_MAX, INT_MAX);

        undo_move();

        if(eval > best_eval){
            best_move = curr_move;
            best_eval = eval;
        }
    
    }

    print_move(best_move);
    printf("\n");

    free_da(&moves_array);

}


