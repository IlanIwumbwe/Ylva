#include "../headers/uci.h"

#ifdef DEV
static void run_test_suite(board_state* state){
    int d, result;
    for(size_t i = 0; i < N_POSITIONS; ++i){
        setup_state_from_fen(state, ts[i].fen);

        d = 0;
        print_board(state);

        while((d < MAX_SEARCH_DEPTH) && (result = ts[i].results[d])){
            printf("Perft test to depth %d\n", d);
            printf("================================\n");
            run_perft(state, d, 0);
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
    fflush(stdout);

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

    state->data->ply = 0; // reset ply to zero such that it is always zero even if moves arg is passed via uci command. hisply has the total ply
}

static void process_go(board_state* state, const char* uci_command){

    char* copy = strdup(uci_command);
    char* cmd;
    char* arg;
    char* end;

    cmd = strtok(copy, " ");

    U64 time = 0, inc = 0;
    int movestogo = 30;

    search_info info = {.maxdepth = MAX_SEARCH_DEPTH, .nodes_searched = 0};

    assert(state != NULL);
    assert(state->data != NULL);

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

static void quit_program(board_state* state){
    free_pv(&state->pvt);
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
            case QUIT: quit_program(&state); goto stop;

        #ifdef DEV
            case PERFT:

                if(!strcmp(uci_command, "perft")){
                    run_test_suite(&state);

                } else if (!strncmp(uci_command, "perft c", 7)){
                    depth = mini(MAX_SEARCH_DEPTH, strtol(uci_command+8, &end, 10));
                    run_perft(&state, depth, 1);
                } else {
                    depth = mini(MAX_SEARCH_DEPTH, strtol(uci_command+6, &end, 10));
                    run_perft(&state, depth, 0);
                }
                break;

            case PRINT:
                print_board(&state); break;

            case UNDO: undo_move(&state); break;

        #endif

        }
    }

    stop:
}
