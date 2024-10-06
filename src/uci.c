#include "../headers/uci.h"

#ifdef DEV
int movegen_test(int depth){
    if(depth == 0){
        return 1;
    }

    U16 move;

    dynamic_array moves_array;
    init_da(&moves_array, 2*128);

    generate_moves(&moves_array, 0);

    int num_nodes = 0;

    for(size_t i = 0; i < moves_array.used; ++i){
        move = moves_array.array[i];

        make_move(move);
        num_nodes += movegen_test(depth - 1);
        undo_move();
    }

    return num_nodes;

    free_da(&moves_array);
}

void run_perft(int depth){

    if(depth){
        U16 move;

        dynamic_array moves_array;
        init_da(&moves_array, 2*128);

        generate_moves(&moves_array, 0);
        int num_nodes = 0, total_nodes = 0;

        for(size_t i = 0; i < moves_array.used; ++i){
            move = moves_array.array[i];

            print_move(move);

            make_move(move);
            num_nodes = movegen_test(depth - 1);
            total_nodes += num_nodes;

            printf(": %d\n", num_nodes);

            undo_move();
        }

        printf("total: %d\n", total_nodes);

        free_da(&moves_array);
    }
}
#endif


static int hash_first_token(const char* uci_command){
    int hash = 0;
    int len = 1;

    while(*uci_command && (*uci_command != ' ')){
        hash ^= (*uci_command++) ^ (len++);
    }

    return hash;
}

static int get_input(char* buffer){
    memset(buffer, 0, INPUT_SIZE);

    if(fgets(buffer, INPUT_SIZE, stdin) == NULL){
        return -1;
    }

    buffer[strcspn(buffer, "\r\n")] = '\0';

    return 0;
}

static void process_position(char* uci_command){
    char* fen_end = strstr(uci_command, "moves");

    int isfen = !strncmp(uci_command, "position fen", 12);

    setup_state_from_fen(isfen ? uci_command + 13 : STARTING_FEN);

    if(fen_end == NULL) return;

    // make moves on board
    char* t = strtok(fen_end, " ");
    U16 move;

    while((t = strtok(NULL, " "))){
        move = move_from_str(t);
        printf("%x\n", move);
        make_move(move);
    }
}


void uci_communication(){
    // perft 5
    // go depth 4
    // uci
    // position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 moves .....
    // position startpos

    populate_attack_sets();

    char uci_command[INPUT_SIZE];
    int depth;

    while(!get_input(uci_command)){
        int h = hash_first_token(uci_command);
        char* end;
        
        printf("%d\n", h);

        switch(h){
            case POSITION: process_position(uci_command); break;


        #ifdef DEV
            case PERFT:
                depth = strtol(uci_command+6, &end, 10);
                run_perft(depth);
                break;

            case PRINT:
                print_board(); break;
        #endif

        
        }
    }

}
