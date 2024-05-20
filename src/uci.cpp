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

/// Just the ones I've implemented 
bool is_valid_go_param(std::string token){
    return (token == "wtime") || (token == "btime") || (token == "winc") || (token == "binc") || (token == "depth") || (token == "searchmoves")
    || (token == "perft");
}

int Uci::movegen_test(int depth){
    std::vector<Move> moves = movegen->generate_moves();
    
    if(depth == 0){
        return 1;
    }

    int num_nodes = 0;    

    for(auto move : moves){
        board->make_move(move);
        num_nodes += movegen_test(depth-1);                               
        board->undo_move();
    }

    return num_nodes;
}

void Uci::perft_driver(int& depth){
    int num_pos = 0, total_pos = 0;
    auto start = high_resolution_clock::now();

    // run perft on starting from legal moves in positionn loaded in by position command
    for(auto move : moves_to_search){
        board->make_move(move);
        num_pos = movegen_test(depth-1);
        board->undo_move();

        std::cout << move << " " << std::dec << num_pos << std::endl;
        total_pos += num_pos;
    }

    auto end = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>(end-start);

    std::cout << "nodes " << std::to_string(total_pos) << std::endl;

    std::cout << "time taken " << std::to_string(duration.count()) << " ms" << std::endl;
    std::cout << "nodes per second " << std::to_string(trunc(total_pos / (duration.count() / 1000.0))) << " nodes/sec" << std::endl;
    std::cout << "\n";
}

void Uci::uci_communication(){
    while(run){
        std::getline(std::cin, input);
        input_size = input.size();

        tokens = get_tokens(input, UCI_COMMAND_FORMAT);

        /*
        for(auto i : tokens){
            std::cout << i << std::endl;
        }*/

        assert(tokens.size() > 0);

        std::string first = tokens[0];

        if(first == "uci"){
            process_uci();
        } else if(first == "position"){
            position_got = true;
            process_position();
        } else if(first == "ucinewgame"){
            board->clear_bitboards();
            position_got = false;        
        } else if(first == "quit"){
            run = false;
        } else if(first == "isready"){
            process_isready();
        } else if(first == "debug"){
            process_debug();
        } else if(first == "print"){
            board->view_board();
        } else if(first == "go" && position_got){
            process_go();
        }

        pointer = 0;
        input = "";

    }
}

void Uci::process_isready(){
    // init pv table, pass size in bytes for the table
    init_pv_table(&board->pv_table, 0x400000);
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
    std::cout << "id name Ylva\n";
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

        board->init_from_fen(fen);  
        movegen->set_state(board); 
        moves_to_search = movegen->generate_moves();

        if(current_token() == "moves"){
            pointer ++; // move over "moves"

            while(current_token() != ""){
                token_s_arg = tokens[pointer];
                
                std::tuple<std::string, std::string, std::string> str_move = parse_player_move(token_s_arg);

                if(convert_to_move(str_move, movegen, _move) == 0){
                    board->make_move(_move);
                    movegen->generate_moves();
                } else {
                    break;
                }
                pointer++;
            }

            moves_to_search = movegen->get_legal_moves();
        }
    }
}

void Uci::process_go(){
    std::vector<std::string> moves;
    std::string curr_token;

    pointer++; // move over "go"

    curr_token = current_token();

    while((curr_token != "") && (is_valid_go_param(curr_token))){
        pointer++; // prepare argument, technically here curr_token is the previous token

        if(current_token() == ""){break;} // no argument

        if(curr_token == "wtime"){
            std::cout << curr_token << " not implemented yet " << std::endl;
        } else if (curr_token == "btime"){
            std::cout << curr_token << " not implemented yet " << std::endl;
        } else if(curr_token == "binc"){
            std::cout << curr_token << " not implemented yet " << std::endl;
        } else if (curr_token == "winc"){  
            std::cout << curr_token << " not implemented yet " << std::endl;
        } else if(curr_token == "depth"){
            std::cout << curr_token << " not implemented yet " << std::endl;
        } else if(curr_token == "searchmoves"){
            
        } else if(curr_token == "perft" && is_valid_num(current_token())){
            token_i_arg = std::stoi(current_token());
            perft_driver(token_i_arg);
        }

        pointer++; // move over argument
        curr_token = current_token();
    }
}



