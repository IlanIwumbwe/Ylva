#include "../headers/perft.h"

#ifdef DEV
U64 movegen_test(board_state* state, int depth){
    if(depth == 0){
        return 1;
    }

    U16 move;

    moves_array legal_moves = {.used = 0};
    generate_moves(state, &legal_moves, 0);

    U64 num_nodes = 0;

    for(size_t i = 0; i < legal_moves.used; ++i){
        move = legal_moves.array[i].move;

        make_move(state, move);

        num_nodes += movegen_test(state, depth - 1);

        undo_move(state);
    }

    return num_nodes;
}

void run_perft(board_state* state, int depth){

    if(depth){
        U16 move;

        U64 start_time = time_in_ms();

        moves_array legal_moves = {.used = 0};
        generate_moves(state, &legal_moves, 0);

        U64 num_nodes = 0ULL, total_nodes = 0ULL;

        for(size_t i = 0; i < legal_moves.used; ++i){
            move = legal_moves.array[i].move;

            print_move(move);
            make_move(state, move);

            num_nodes = movegen_test(state, depth - 1);
            total_nodes += num_nodes;

            printf(": %ld\n", num_nodes);

            undo_move(state);

        }

        U64 end_time = time_in_ms();
        U64 time = end_time - start_time;

        printf("total: %ld\n", total_nodes);

        if(time / 1000) printf("nps: %ld\n", (total_nodes * 1000) / time);
    } else {
        printf("total: 1\n");
    }
}
#endif

