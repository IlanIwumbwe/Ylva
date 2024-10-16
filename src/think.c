#include "../headers/think.h"

// attackers :  p, k, n, b, r, q, none

const int MVV_LVA[7][7] = {
    {15, 10, 14, 13, 12, 11, 0}, // victim = p 
    {0, 0, 0, 0, 0, 0, 0},   // victim = k
    {25, 20, 24, 23, 22, 21, 0}, // victim = n
    {35, 30, 34, 33, 32, 31, 0}, // victim = b
    {45, 40, 44, 43, 42, 41, 0},  // victim = r
    {55, 50, 54, 53, 52, 51, 0}, // victim = q
    {0, 0, 0, 0, 0, 0, 0}, // victim = none
};

static void check_stop_conditions(search_info* info){
    if((info->time_set && (time_in_ms() > info->end_time)) || input_waiting()){
        info->stopped = 1;
    }
}

static int eval(){
    int perspective = (board_info->s == WHITE) ? 1 : -1;
    return perspective * (board_info->material[WHITE] - board_info->material[BLACK]);
}

/// @brief Find the move with the highest score and put it at the current iteration index
/// @param moves_array 
/// @param current_move_index 
static void pick_move(dynamic_array* moves_array, int current_move_index){
    Move tmp;
    Move curr_move;

    for(size_t i = current_move_index+1; i < moves_array->used; ++i){
        curr_move = moves_array->array[i];
        
        if(curr_move.score > moves_array->array[current_move_index].score){
            // swap
            tmp = moves_array->array[current_move_index];
            moves_array->array[current_move_index] = curr_move;
            moves_array->array[i] = tmp;
        }
    }

}

/// @brief plain negamax without any optimisations
/// @param depth 
/// @return 
static int negamax_search(int depth){

    if(depth == 0){
        return eval();
    }

    Move move;
    int eval = 0, max_eval = -INT_MAX;

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

        make_move(move.move);

        eval = -negamax_search(depth - 1);

        undo_move();

        if(eval > max_eval){
            max_eval = eval;
        }
    }

    free_da(&moves_array);
    
    return max_eval;
}


/// @brief check whether move is valid in the current board position
/// @param move 
/// @return 
int move_is_valid(U16 move){

    dynamic_array moves_array;
    init_da(&moves_array, 218);

    generate_moves(&moves_array, 0);

    for(size_t i = 0; i < moves_array.used; ++i){
        if(move == moves_array.array[i].move) return 1;
    }

    return 0;

    free_da(&moves_array);
}

/// @brief Get pv line to certain depth
/// @param depth 
int get_pv_line(int depth){
    assert(depth < MAX_SEARCH_DEPTH);

    U16 move = probe_pv_table(&pvt, board_info->hash);
    int count = 0;

    while((move != 0) && (count < depth)){
        
        if(move_is_valid(move)){
            make_move(move);
            pv_array[count++] = move;
        } else {
            break;
        }

        move = probe_pv_table(&pvt, board_info->hash);
    }

    // reset to original position
    while(board_info->ply != 0){
        undo_move();
    }

    return count;
}

/// @brief Score pv move very highly, set scores of all other moves to 0
/// @param da 
void sort_pv_move(dynamic_array* moves_array){

    Move* curr_move;

    for(size_t i = 0; i < moves_array->used; ++i){
        curr_move = moves_array->array + i;

        if(curr_move->move == pv_array[board_info->ply]){ curr_move->score = 100; } 
        else { curr_move->score = 0; }
    }
}

void order_moves(dynamic_array* moves_array){

    Move* curr_move;
    piece p_to;
    piece p_from;

    for(size_t i = 0; i < moves_array->used; ++i){
        curr_move = moves_array->array + i;

        p_to = piece_on_square(move_to_square(curr_move->move));
        p_from = piece_on_square(move_from_square(curr_move->move));

        // give good captures a high score than bad captures
        curr_move->score += MVV_LVA[p_to - (6 & -(p_to > 5))][p_from - (6 & -(p_from > 5))];
    }

}

/// @brief negamax search on the position with alpha-beta pruning and move ordering
/// @param depth 
/// @param alpha 
/// @param beta 
/// @return 
static int search(int depth, int alpha, int beta, search_info* info){
    if(depth == 0){
        return eval(); 
    }

    if(info->nodes_searched & 2047){
        check_stop_conditions(info);
    }

    Move move;
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

    sort_pv_move(&moves_array);
    order_moves(&moves_array);  

    for(size_t i = 0; i < moves_array.used; ++i){
        pick_move(&moves_array, i);
        move = moves_array.array[i];

        if(info->stopped){
            return 0;
        }

        make_move(move.move);

        info->nodes_searched += 1;
        eval = -search(depth - 1, -beta, -alpha, info);
        
        undo_move();

        if(eval > alpha){
            store_pv_entry(&pvt, board_info->hash, move.move);
            alpha = eval;

            if(eval >= beta){
                return beta; // beta cutoff
            }
        }
    }

    free_da(&moves_array);
    
    return alpha;
}

void think(search_info* info){
    Move curr_move;

    int best_eval = -INT_MAX;
    int pv_len = 0;

    dynamic_array moves_array;
    init_da(&moves_array, 218);

    generate_moves(&moves_array, 0);

    for(int d = 1; d <= info->maxdepth; ++d){
        info->nodes_searched = 0;

        sort_pv_move(&moves_array);
        order_moves(&moves_array);        

        for(size_t i = 0; i < moves_array.used; ++i){
            pick_move(&moves_array, i);
            curr_move = moves_array.array[i];

            make_move(curr_move.move);

            info->nodes_searched += 1;
            curr_move.score = -search(d-1, -INT_MAX, INT_MAX, info);  // get "true" eval from search

            undo_move();

            if(curr_move.score > best_eval){
                store_pv_entry(&pvt, board_info->hash, curr_move.move);
                best_eval = curr_move.score;
            }
        }

        pv_len = get_pv_line(d);

        printf("info depth %d nodes %d pv ", d, info->nodes_searched);

        for(int i = 0; i < pv_len; i++){
            print_move(pv_array[i]);
        }

        printf("\n");

        //reset_pv_entries(&pvt);

        if(info->stopped){break;}
    }

    free_da(&moves_array);

    clear_pv_array(pv_array);

}


