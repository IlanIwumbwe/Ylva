#include "../headers/think.h"

int eval(){
    int perspective = (board_info->s == WHITE) ? 1 : -1;
    return perspective * (board_info->material[WHITE] - board_info->material[BLACK]);   
}

int search(int depth){

    if(depth == 0){
        return eval(); 
    }

    U16 move;
    int max_eval = -INT_MAX;
    int eval;

    dynamic_array moves_array;
    init_da(&moves_array, 218);

    generate_moves(&moves_array, 0);

    for(size_t i = 0; i < moves_array.used; ++i){
        move = moves_array.array[i];

        make_move(move);

        eval = -search(depth - 1);

        undo_move();

        max_eval = maxi(max_eval, eval);
    }

    free_da(&moves_array);
    
    return max_eval;

}

void think(int depth){

    dynamic_array moves_array;
    init_da(&moves_array, 218);

    generate_moves(&moves_array, 0);

    U16 curr_move;
    U16 best_move = 0;
    int best_eval = 0;
    int eval = 0;

    for(size_t i = 0; i < moves_array.used; ++i){
        curr_move = moves_array.array[i];

        make_move(curr_move);

        eval = -search(depth-1);

        undo_move();

        if(eval > best_eval){
            best_move = curr_move;
            best_eval = eval;
        }
    
    }

    printf("Best move %x \n", best_move);

    free_da(&moves_array);

}


