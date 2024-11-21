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

static int eval(board_state* state){
    int perspective = (state->data->s == WHITE) ? 1 : -1;
    return perspective * (state->data->eval[WHITE] - state->data->eval[BLACK]);
}

/// @brief Find the move with the highest score and put it at the current iteration index
/// @param moves_array 
/// @param current_move_index 
static void pick_move(moves_array* legal_moves, int current_move_index){
    Move tmp;
    Move curr_move;

    for(size_t i = current_move_index+1; i < legal_moves->used; ++i){
        curr_move = legal_moves->array[i];
        
        if(curr_move.score > legal_moves->array[current_move_index].score){
            // swap
            tmp = legal_moves->array[current_move_index];
            legal_moves->array[current_move_index] = curr_move;
            legal_moves->array[i] = tmp;
        }
    }

}

/// @brief plain negamax without any optimisations
/// @param depth 
/// @return 
static int negamax_search(int depth, search_info* info){

    if(depth == 0){
        return eval(info->state);
    }

    Move move;
    int eval = 0, max_eval = -INT_MAX;

    moves_array legal_moves = {.used=0};
    generate_moves(info->state, &legal_moves, 0);

    if(legal_moves.used == 0){
        if(n_checkers){
            return -INT_MAX; // checkmate
        } else {
            return 0;  // stalemate
        }
        
    }

    for(size_t i = 0; i < legal_moves.used; ++i){
        move = legal_moves.array[i];

        make_move(info->state, move.move);
        info->nodes_searched += 1;

        eval = -negamax_search(depth - 1, info);

        undo_move(info->state);

        if(eval > max_eval){
            max_eval = eval;
        }
    }
    
    return max_eval;
}


/// @brief check whether move is valid in the current board position
/// @param move 
/// @return 
static int move_is_valid(board_state* state, U16 move){

    moves_array legal_moves = {.used=0};
    generate_moves(state, &legal_moves, 0);

    for(size_t i = 0; i < legal_moves.used; ++i){
        if(move == legal_moves.array[i].move) return 1;
    }

    return 0;
}

/// @brief Get pv line to certain depth
/// @param depth 
static int get_pv_line(board_state* state, int depth){
    assert(depth < MAX_SEARCH_DEPTH);

    U16 move = probe_pv_table(state);
    int count = 0;

    while((move != 0) && (count < depth)){
        
        if(move_is_valid(state, move)){
            make_move(state, move);
            state->pv_array[count++] = move;
        } else {
            break;
        }

        move = probe_pv_table(state);
    }

    // reset to original position
    while(state->data->ply != 0){
        undo_move(state);
    }

    return count;
}

/// @brief Score pv move very highly, set scores of all other moves to 0
/// @param da 
static void sort_pv_move(board_state* state, moves_array* legal_moves){

    Move* curr_move;

    for(size_t i = 0; i < legal_moves->used; ++i){
        curr_move = legal_moves->array + i;

        if(curr_move->move == state->pv_array[state->data->ply]){ curr_move->score = 100; } 
        else { curr_move->score = 0; }
    }
}

/// @brief Use MVV_LVA heuristic to order moves
/// @param legal_moves 
static void order_moves(board_state* state, moves_array* legal_moves){

    Move* curr_move;
    piece p_to;
    piece p_from;

    for(size_t i = 0; i < legal_moves->used; ++i){
        curr_move = legal_moves->array + i;

        p_to = state->board[move_to_square(curr_move->move)];
        p_from = state->board[move_from_square(curr_move->move)];

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
        return eval(info->state); 
    }

    if(info->nodes_searched & 2047){
        check_stop_conditions(info);
    }

    Move move;
    int eval;

    moves_array legal_moves = {.used=0};
    generate_moves(info->state, &legal_moves, 0);

    if(legal_moves.used == 0){
        if(n_checkers){
            return -INT_MAX; // checkmate
        } else {
            return 0;  // stalemate
        }
        
    }

    sort_pv_move(info->state, &legal_moves); // score pv move highly, also sets all other move scores to 0
    order_moves(info->state, &legal_moves);  

    for(size_t i = 0; i < legal_moves.used; ++i){
        pick_move(&legal_moves, i);
        move = legal_moves.array[i];

        if(info->stopped){
            return 0;
        }

        make_move(info->state, move.move);

        info->nodes_searched += 1;
        eval = -search(depth - 1, -beta, -alpha, info);
        
        undo_move(info->state);

        if(eval > alpha){
            store_pv_entry(info->state, move.move);
            alpha = eval;

            if(eval >= beta){
                return beta; // beta cutoff
            }
        }
    }
    
    return alpha;
}

void think(search_info* info){
    Move curr_move;

    int best_eval = -INT_MAX;
    int pv_len = 0;

    moves_array legal_moves = {.used=0};
    generate_moves(info->state, &legal_moves, 0);

    for(int d = 1; d <= info->maxdepth; ++d){
        sort_pv_move(info->state, &legal_moves); // score pv move highly, also sets all other move scores to 0
        order_moves(info->state, &legal_moves);        

        for(size_t i = 0; i < legal_moves.used; ++i){
            pick_move(&legal_moves, i);
            curr_move = legal_moves.array[i];

            make_move(info->state, curr_move.move);

            info->nodes_searched += 1;
            curr_move.score = -search(d-1, -INT_MAX, INT_MAX, info);  // get "true" eval from search

            undo_move(info->state);

            if(curr_move.score > best_eval){
                store_pv_entry(info->state, curr_move.move);
                best_eval = curr_move.score;
            }
        }

        pv_len = get_pv_line(info->state, d);

        printf("info depth %d nodes %d pv ", d, info->nodes_searched);

        for(int i = 0; i < pv_len; i++){
            print_move(info->state->pv_array[i]);
        }

        printf("\n");

        if(info->stopped){break;}
    }
}


