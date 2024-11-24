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

static int static_eval(board_state* state){
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
static int negamax_search(int depth, search_info* info, board_state* state){

    if(depth == 0){
        return eval(state);
    }

    Move move;
    int eval = 0, max_eval = -INT_MAX;

    moves_array legal_moves = {.used=0};
    generate_moves(state, &legal_moves, 0);

    if(legal_moves.used == 0){
        if(state->n_checkers){
            return -INT_MAX; // checkmate
        } else {
            return 0;  // stalemate
        }
        
    }

    if(state->data->fifty_move >= 100){
        return 0; // stalemate by fifty move rule
    }

    for(size_t i = 0; i < legal_moves.used; ++i){
        move = legal_moves.array[i];

        make_move(state, move.move);
        info->nodes_searched += 1;

        eval = -negamax_search(depth - 1, info, state);

        undo_move(state);

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
    int old_ply = state->data->ply;

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
    while(state->data->ply != old_ply){
        undo_move(state);
    }

    return count;
}

/// @brief Give hueristic score to moves such that moves are searched in this order: pv move, good captures, killer moves, history moves
/// @param legal_moves 
static void order_moves(const board_state* state, moves_array* legal_moves){

    Move* curr_move;
    piece p_to, p_from;
    square s_to, s_from;

    for(size_t i = 0; i < legal_moves->used; ++i){
        curr_move = legal_moves->array + i;

        s_to = move_to_square(curr_move->move);
        s_from = move_from_square(curr_move->move);
        p_to = state->board[s_to];
        p_from = state->board[s_from];

        // if pv move, bump up score
        if(curr_move->move == state->pv_array[state->data->ply]){ 
            curr_move->score = PV_SCORE;
        }

        // give good captures a high score than bad captures using MVV_LVA heuristic
        curr_move->score += MVV_LVA[p_to - (6 & -(p_to > 5))][p_from - (6 & -(p_from > 5))] + CAPTURE_SCORE_OFFSET;

        // check if current move is a killer move
        if(state->killer_moves[0][state->data->ply] == curr_move->move){
            curr_move->score += KILLER_0_SCORE;
        } else if(state->killer_moves[1][state->data->ply] == curr_move->move){
            curr_move->score += KILLER_1_SCORE;
        } else {
            // history move
            curr_move->score += state->history_moves[s_from][s_to];
        }
    
    }
}

/// @brief negamax search on the position with alpha-beta pruning and move ordering
/// @param depth 
/// @param alpha 
/// @param beta 
/// @return 
static int search(int depth, int alpha, int beta, search_info* info, board_state* state){
    if(info->nodes_searched & 2047){
        check_stop_conditions(info);
    }

    Move move, best_move = {.move = 0, .score = 0};
    int eval;
    int old_alpha = alpha;

    moves_array legal_moves = {.used=0};
    generate_moves(state, &legal_moves, 0);

    if(legal_moves.used == 0){
        if(state->n_checkers){
            return -INT_MAX; // checkmate
        } else {
            return 0;  // stalemate
        }
        
    }

    if(state->data->fifty_move >= 100){
        return 0; // stalemate by fifty move rule
    }

    if(depth == 0){
        return static_eval(state); 
    }

    order_moves(state, &legal_moves);  

    for(size_t i = 0; i < legal_moves.used; ++i){
        pick_move(&legal_moves, i);
        move = legal_moves.array[i];

        if(info->stopped){
            return 0;
        }

        make_move(state, move.move);

        info->nodes_searched += 1;
        eval = -search(depth - 1, -beta, -alpha, info, state);
        
        undo_move(state);

        if(eval > alpha){
            //printf("move ");
            //print_move(move.move);
            //printf(", eval %d, alpha %d\n", eval, alpha);

            alpha = eval;
            best_move = move;

            store_pv_entry(state, best_move.move);

            if(eval >= beta){
                
                // non-capture moves that cause beta cutoffs are killer moves
                if(!(move.move & CAPTURE_MASK)){
                    state->killer_moves[1][state->data->ply] = state->killer_moves[0][state->data->ply];
                    state->killer_moves[0][state->data->ply] = move.move;
                }

                return beta; // beta cutoff
            }

            // non-capture moves that improve upon alpha increment history score
            if(!(move.move & CAPTURE_MASK)){
                state->history_moves[move_from_square(best_move.move)][move_to_square(best_move.move)] += depth;
            }
        }
    }   
    
    return alpha;
}

void think(search_info* info, board_state* state){

    int best_eval = -INT_MAX;
    int pv_len = 0;

    U16 best_move = 0;

    for(int d = 1; d <= info->maxdepth; ++d){  

        best_eval = search(d, -AB_BOUND, AB_BOUND, info, state);

        if(info->stopped){break;}

        pv_len = get_pv_line(state, d);

        printf("info depth %d nodes %d pv ", d, info->nodes_searched);

        for(int i = 0; i < pv_len; i++){
            print_move(state->pv_array[i]);
        }

        printf("\n");
        info->nodes_searched = 0;
    }

    if(pv_len){
        best_move = state->pv_array[0];
        printf("best move ");
        print_move(best_move);
        printf("\n");
    }

    reset_state(state);
}


