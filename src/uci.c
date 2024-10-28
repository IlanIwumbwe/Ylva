#include "../headers/uci.h"

#ifdef DEV
U64 movegen_test(int depth){
    if(depth == 0){
        return 1;
    }

    U16 move;

    moves_array legal_moves = {.used = 0};
    generate_moves(&legal_moves, 0);

    U64 num_nodes = 0;

    for(size_t i = 0; i < legal_moves.used; ++i){
        move = legal_moves.array[i].move;

        make_move(move);

        num_nodes += movegen_test(depth - 1);

        undo_move();
    }

    return num_nodes;
}

void run_perft(int depth){

    if(depth){
        U16 move;

        U64 start_time = time_in_ms();

        moves_array legal_moves = {.used = 0};
        generate_moves(&legal_moves, 0);

        U64 num_nodes = 0ULL, total_nodes = 0ULL;

        for(size_t i = 0; i < legal_moves.used; ++i){
            move = legal_moves.array[i].move;

            print_move(move);
            make_move(move);

            num_nodes = movegen_test(depth - 1);
            total_nodes += num_nodes;

            printf(": %ld\n", num_nodes);

            undo_move();

        }

        U64 end_time = time_in_ms();
        U64 time = end_time - start_time;

        printf("total: %ld\n", total_nodes);

        if(time / 1000) printf("nps: %ld\n", (total_nodes * 1000) / time);
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

static void process_uci(){
    printf("id name ylva \nid author Ilan \nuciok\n");
}

static void process_isready(){
    printf("readyok\n");
}

static void process_position(const char* uci_command){
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

void process_go(const char* uci_command){
    
    char* copy = strdup(uci_command);
    char* cmd;
    char* arg;
    char* end;

    cmd = strtok(copy, " ");

    U64 time = 0, inc = 0;
    int movestogo = 30;

    search_info info = {.maxdepth = MAX_SEARCH_DEPTH, .nodes_searched = 0};

    while((cmd = strtok(NULL, " ")) && (arg = strtok(NULL, " "))){
        if(!strcmp(cmd, "depth")){
            info.maxdepth = mini(MAX_SEARCH_DEPTH, strtol(arg, &end, 10));
        } else if (!strcmp(cmd, "wtime") && (board_info->s == WHITE)){
            time = strtoll(arg, &end, 10);
        } else if (!strcmp(cmd, "btime") && (board_info->s == BLACK)) {
            time = strtoll(arg, &end, 10);
        } else if (!strcmp(cmd, "winc") && (board_info->s == WHITE)) {
            inc = strtoll(arg, &end, 10);
        } else if (!strcmp(cmd, "binc") && (board_info->s == BLACK)) {
            inc = strtoll(arg, &end, 10);
        } else if(!strcmp(cmd, "movetime")){
            time = strtoll(arg, &end, 10);
            movestogo = 1;
        } else if(!strcmp(cmd, "movestogo")){   
            movestogo = strtol(arg, &end, 10);
        } 
    }

    if(time != 0){
        info.start_time = time_in_ms();
        time /= movestogo;
        info.end_time = info.start_time + inc + time;
        info.time_set = 1;
    }

    printf("start_time %ld end_time %ld depth %d time to think %ld inc %ld timeset %d\n", info.start_time, info.end_time, info.maxdepth, time, inc, info.time_set);

    think(&info);

}

void uci_communication(){
    // perft 5
    // go depth 4
    // uci
    // position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 moves .....
    // position startpos

    populate_attack_sets();
    init_hash_keys();

    char uci_command[INPUT_SIZE];
    int depth;

    while(!get_input(uci_command)){
        int h = hash_first_token(uci_command);
        char* end;
        
        //printf("%d\n", h);

        switch(h){
            case POSITION: process_position(uci_command); break;
            case GO: process_go(uci_command); break;
            case UCI: process_uci(); break;
            case ISREADY: process_isready(); break;
        
        #ifdef DEV
            case PERFT:
                depth = mini(MAX_SEARCH_DEPTH, strtol(uci_command+6, &end, 10));
                run_perft(depth);
                break;

            case PRINT:
                print_board(); break;

            case UNDO: undo_move(); break;

        #endif

        }
    }

    free_pv(&pvt);
}
