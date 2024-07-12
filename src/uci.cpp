#include "uci.h"

/// @brief Convert tuple of strings into move object, and check move validity
/// @param str_move 
/// @param movegen 
/// @param move 
/// @return 0 if move is valid, -1 if not
int convert_to_move(const std::tuple<std::string, std::string, std::string>& str_move, MoveGen* movegen, Move& move){
    uint from, to, flags;

    auto [from_str, to_str, promo_piece] = str_move;

    int diff = 0;

    from = alg_to_int(from_str);
    to = alg_to_int(to_str);

    U64 from_bitboard = set_bit(from), to_bitboard = set_bit(to);

    diff = from - to;

    if(promo_piece == " "){
        // no promotion
        if(movegen->occupied & to_bitboard){
            flags = 4;
        } else {
            if(movegen->pawns & from_bitboard){
                if(abs(diff) == 9 || abs(diff) == 7){
                    flags = 5;
                } else if(diff == 16 || diff == -16){
                    flags = 1;
                } else {
                    flags = 0;
                }
            } else if((movegen->ally_king | movegen->enemy_king) & from_bitboard){
                if(diff == 2){
                    flags = 2;
                } else if(diff == -2){
                    flags = 3;
                } else {
                    flags = 0;
                } 
            } else {
                flags = 0;
            }
        }

    } else {
        // wants to promote
        flags = (movegen->occupied & to_bitboard) ? promo_flags[promo_piece+"c"] : promo_flags[promo_piece];
    }

    move = Move(from,to,flags);

    return !movegen->move_is_legal(move);
}

/// @brief Parse move and return tuple
/// @param str_move 
/// @return from, to, promotion_piece
std::tuple<std::string, std::string, std::string> parse_player_move(std::string& str_move){
    auto move_size = str_move.size();
    std::string from(1, str_move[0]);
    from += str_move[1];

    std::string to(1,str_move[2]);
    to += str_move[3];

    std::string promo_piece = " ";

    if(move_size == 5){promo_piece = str_move[4];}

    return std::make_tuple(from, to, promo_piece);
}

int Uci::movegen_test(int depth){

    if(depth == 0){
        return 1;
    } else {
        std::vector<Move> moves = movegen->generate_moves();
        int num_nodes = 0;    

        for(auto move : moves){
            board->make_move(move);
            num_nodes += movegen_test(depth-1);                               
            board->undo_move();
        }

        return num_nodes;
    }
}

void Uci::perft_driver(int& depth){
    int num_pos = 0, total_pos = 0;
    U64 start = time_in_ms();

    // run perft on starting from legal moves in positionn loaded in by position command
    for(auto move : moves_to_search){
        board->make_move(move);
        num_pos = movegen_test(depth-1);
        board->undo_move();

        std::cout << move << " " << std::dec << num_pos << std::endl;
        total_pos += num_pos;
    }

    U64 end = time_in_ms();

    U64 duration = end-start;

    std::cout << "nodes " << std::to_string(total_pos) << std::endl;

    std::cout << "time taken " << std::to_string(duration) << " ms" << std::endl;
    std::cout << "nodes per second " << std::to_string(trunc(total_pos / (duration / 1000.0))) << " nodes/sec" << std::endl;
    std::cout << "\n";
}

void Uci::uci_communication(){
    while(run){
        std::getline(std::cin, input);
        input_size = input.size();

        tokens = get_tokens(input, UCI_COMMAND_FORMAT);

        if(input_size > 0){
            std::string first = current_token();

            if(first == "uci"){
                process_uci();
            } else if(first == "position"){
                process_position();
            } else if(first == "ucinewgame"){
                process_ucinewgame();    
            } else if(first == "quit"){
                run = false;
            } else if(first == "isready"){
                process_isready();
            } else if(first == "debug"){
                process_debug();
            } else if(first == "print"){
                board->view_board();
                /*
                for(auto move : movegen->get_legal_moves()){
                    std::cout << move << std::endl;
                }
                */

            } else if(first == "go"){
                process_go();
            } 

            pointer = 0;
            input = "";
        } else if (newgameset && engine->quit){
            // quit from engine think interrupt
            run = false;
        }
    }
}

void Uci::process_ucinewgame(){
    board->clear_bitboards();
    // init pv table, pass size in bytes for the table
    init_pv_table(&(board->pv_table), 0x4000000);
    engine = std::make_shared<Enginev2>(board, movegen);

    // use the magic numbers that were pregenerated to precalculate move attack sets
    U64 occupancy, blockers;
    int key, square, i;
    
    for (square = 0; square < 64; ++square){
        occupancy = movegen_helpers::get_bishop_occupancies(square);

        for(i = 0; i < (1 << BBits[square]); i++){
            blockers = movegen_helpers::get_blocker_config(i, BBits[square], occupancy);
            key = movegen_helpers::transform_to_key(blockers, bishop_magics[square], BBits[square]);
            bishop_moves[square][key] = movegen_helpers::get_bishop_attacks(square, blockers);
        }
    }

    for (square = 0; square < 64; ++square){
        occupancy = movegen_helpers::get_rook_occupancies(square);
        
        for(int i = 0; i < (1 << RBits[square]); i++){
            blockers = movegen_helpers::get_blocker_config(i, RBits[square], occupancy);
			key = movegen_helpers::transform_to_key(blockers, rook_magics[square], RBits[square]);
            rook_moves[square][key] = movegen_helpers::get_rook_attacks(square, blockers);
        }
    }

    newgameset = true;
}

void Uci::process_isready(){
    std::cout << "readyok\n";
}

void Uci::process_debug(){
    pointer ++; // move over "debug" 

    std::string next = current_token();

    if(next == "on"){
        debug = true;
    } else if(next == "off"){
        debug = false;
    }

}

void Uci::process_uci(){
    std::cout << "id name "<< engine_name << std::endl;
    std::cout << "id author Ilan Iwumbwe\n";

    // send_options
    std::cout << "uciok\n";

}

void Uci::process_position(){
    std::string fen, curr_token;
    pointer ++; // move over "position"

    curr_token = current_token();

    if((curr_token == "startpos") || (curr_token == "fen")){

        if(curr_token == "startpos"){
            fen = STARTING_FEN;
        } else {
            pointer++; // point to fen string
            fen = current_token();
        } 
        
        pointer++; // move over <fen>

        board->clear_bitboards();
        board->init_from_fen(fen);  
        movegen->set_state(board); 
        moves_to_search = movegen->generate_moves();

        if(current_token() == "moves"){
            pointer ++; // move over "moves"

            while(current_token() != ""){
                token_s_arg = tokens[pointer++];
                
                std::tuple<std::string, std::string, std::string> str_move = parse_player_move(token_s_arg);

                if(convert_to_move(str_move, movegen, _move) == 0){
                    board->make_move(_move);
                    movegen->generate_moves();
                } else {
                    std::cout << token_s_arg << std::endl;
                    for (auto m : moves_to_search){
                        std::cout << m << std::endl;
                    }
                    break;
                }
            }

            moves_to_search = movegen->get_legal_moves();
        }

        engine_side = board->get_turn();
    }
}

// implementation of this partly follows Bluefever software's UCI video, part 69
void Uci::process_go(){
    std::vector<std::string> moves;
    std::string curr_token;

    U64 time = 0, inc = 0;
    int max_search_depth = MAX_DEPTH, movestogo = 30, movetime = -1;  

    pointer++; // move over "go"

    curr_token = current_token();

    while(curr_token != ""){
        pointer++; // prepare argument, technically here curr_token is the previous token

        if(current_token() == ""){break;} // no argument

        if(curr_token == "wtime" && engine_side == white){
            time = std::stoi(current_token());
        } else if (curr_token == "btime" && engine_side == black){
            time = std::stoi(current_token());
        } else if(curr_token == "binc" && engine_side == black){
            inc = std::stoi(current_token());
        } else if (curr_token == "winc" && engine_side == white){  
            inc = std::stoi(current_token());
        } else if(curr_token == "depth"){
            max_search_depth = std::stoi(current_token());
        } else if(curr_token == "searchmoves"){
            std::cout << curr_token << " not implemented yet " << std::endl; 
        } else if (curr_token == "infinite"){
            ;
        } else if (curr_token == "movetime"){
            movetime = std::stoi(current_token());
        } else if (curr_token == "movestogo"){
            movestogo = std::stoi(current_token());
        } else if(curr_token == "perft" && is_valid_num(current_token())){
            token_i_arg = std::stoi(current_token());
            perft_driver(token_i_arg);
            return ;
        }

        pointer++; // move over argument
        curr_token = current_token();
    }

    engine->set_depth(max_search_depth);

    if(movetime != -1){
        time = movetime;
        movestogo = 1;
    }

    engine->start_time = time_in_ms();

    if(time != 0){
        engine->time_set = true;
        time /= movestogo;
        time -= 50; // take off 50ms for safety
        engine->stop_time = engine->start_time + time + inc;
    }

    std::cout << std::dec << "time to think: " << time << " starttime: " << engine->start_time << " stoptime: " << engine->stop_time << " depth: " << max_search_depth << " timeset: " << engine->time_set << std::endl;

    // start a search that can be interrupted at any time by "stop"
    engine->engine_driver(moves_to_search);

    engine->pv_length = 0;
    engine->time_set = false;
    engine->stopped = false;
}


