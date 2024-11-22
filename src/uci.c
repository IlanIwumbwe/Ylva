#include "../headers/uci.h"

#ifdef DEV
static U64 movegen_test(board_state* state, int depth){
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

static void run_perft(board_state* state, int depth){

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

static void run_test_suite(board_state* state){
    int d, result;
    for(size_t i = 0; i < N_POSITIONS; ++i){
        setup_state_from_fen(state, ts[i].fen);

        d = 0;
        print_board(state);
        
        while((d < MAX_SEARCH_DEPTH) && (result = ts[i].results[d])){
            printf("Perft test to depth %d\n", d);
            printf("================================\n");
            run_perft(state, d);
            printf("Expected nodes %d\n", result);
            d++;
        }

        printf("\n");
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

/// @brief converts str move into U16
/// @param move 
/// @return 
static U16 move_from_str(board_state* state, char* move){

    square from = sq(move) & 0x3f;
    square to = sq(move+2) & 0x3f;
    int m_type = 0x0;

    piece p_from = state->board[from];
    piece p_to = state->board[to];

    set_promotion_type(move+4, &m_type);

    int square_dist = from - to;
    
    if(p_to != p_none){
        m_type |= 0x4;
    }

    if(m_type) goto end;

    if((p_from == P) || (p_from == p)){
        if(abs(square_dist) == 9 || abs(square_dist) == 7){
            m_type = 0x5;
        } else if(abs(square_dist) == 16){
            m_type = 0x1;
        }
    } else if ((p_from == K) || (p_from == k)){
        if(square_dist == 2){
            m_type = 0x2;
        } else if(square_dist == -2){
            m_type = 0x3;
        }
    }

    end:
    return (m_type << 12) | (from << 6) | to;
}

static void process_uci(){
    printf("id name ylva \nid author Ilan \nuciok\n");
}

static void process_isready(){
    printf("readyok\n");
}

static void process_position(board_state* state, const char* uci_command){
    char* fen_end = strstr(uci_command, "moves");

    int isfen = !strncmp(uci_command, "position fen", 12);

    setup_state_from_fen(state, isfen ? uci_command + 13 : STARTING_FEN);

    init_hash(state);
    init_eval(state);
    init_pv(state, 200000);

    if(fen_end == NULL) return;

    // make moves on board
    char* t = strtok(fen_end, " ");
    U16 move;

    while((t = strtok(NULL, " "))){
        move = move_from_str(state, t);
        make_move(state, move);
    }
}

void process_go(board_state* state, const char* uci_command){
    
    char* copy = strdup(uci_command);
    char* cmd;
    char* arg;
    char* end;

    cmd = strtok(copy, " ");

    U64 time = 0, inc = 0;
    int movestogo = 30;

    search_info info = {.maxdepth = MAX_SEARCH_DEPTH, .nodes_searched = 0};
    side s = state->data->s;

    while((cmd = strtok(NULL, " ")) && (arg = strtok(NULL, " "))){
        if(!strcmp(cmd, "depth")){
            info.maxdepth = mini(MAX_SEARCH_DEPTH, strtol(arg, &end, 10));
        } else if (!strcmp(cmd, "wtime") && (s == WHITE)){
            time = strtoll(arg, &end, 10);
        } else if (!strcmp(cmd, "btime") && (s == BLACK)) {
            time = strtoll(arg, &end, 10);
        } else if (!strcmp(cmd, "winc") && (s == WHITE)) {
            inc = strtoll(arg, &end, 10);
        } else if (!strcmp(cmd, "binc") && (s == BLACK)) {
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

    think(&info, state);
}

void uci_communication(){
    // perft 5
    // go depth 4
    // uci
    // position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 moves .....
    // position startpos

    board_state state;

    populate_attack_sets();
    init_hash_keys();
    init_flip();
    
    char uci_command[INPUT_SIZE];
    int depth;

    while(!get_input(uci_command)){
        int h = hash_first_token(uci_command);
        char* end;

        switch(h){
            case POSITION: process_position(&state, uci_command); break;
            case GO: process_go(&state, uci_command); break;
            case UCI: process_uci(); break;
            case ISREADY: process_isready(); break;
        
        #ifdef DEV
            case PERFT:

                if(!strcmp(uci_command, "perft")){
                    run_test_suite(&state);

                } else {
                    depth = mini(MAX_SEARCH_DEPTH, strtol(uci_command+6, &end, 10));
                    run_perft(&state, depth);
                }
                break;

            case PRINT:
                print_board(&state); break;

            case UNDO: undo_move(&state); break;

        #endif

        }
    }
}
