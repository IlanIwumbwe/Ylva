#include "run.h"

Run::Run(std::string& fen, game_modes mode) : board(fen), movegen(&board), mode(mode) {
    // generate moves for the start state (no moves made yet)
    movegen.generate_moves();

    if(mode == PVP){
        run_PVP();
    } else if(mode == PVE){
        int depth = get_perft_depth();
        set_engine(depth);
        run_PVE();
    } else if(mode == EVE) {
        int depth = get_perft_depth();
        set_engine(depth);
        run_EVE();
    } else if(mode == PERFT){
        run_perft();
    } else {
        std::cerr << "Unexpected mode " << mode << std::endl;
    }
}

void Run::run_PVP(){
    while (run){
        get_input_from_player();
        end_game();
    }
}

void Run::run_PVE(){
    std::string colour_choice = get_colour_choice();

    player_side = (colour_choice == "w") ? WHITE : BLACK;

    while(run){
        std::cout << "\nwhite time: " << white_used_time.count() << " seconds" << std::endl; 
        std::cout << "black time: " << black_used_time.count() << " seconds" << std::endl; 

        if(board.get_turn() == player_side){
            get_input_from_player();
        }else{
            engine->engine_driver();

            if(player_side){
                white_used_time += engine->time_used_per_turn;
            } else {
                black_used_time += engine->time_used_per_turn;
            }
        }
        end_game();
    }
}

void Run::run_EVE(){
    while(run){
        engine->engine_driver();
        end_game();
    }
}

void Run::end_game(){
    if(board.get_hm_clock() == 101){
        std::cout << "Draw by 50 move rule" << std::endl;
        board.view_board();
        run = false;
    } else if(movegen.no_legal_moves()){
        board.view_board();
        if(movegen.ally_king_in_check()){
            std::cout << (board.get_turn() ? "White " : "Black ") << "wins by checkmate" << std::endl;
        } else { std::cout << "Draw by stalemate" << std::endl; }
        run = false;
    } else if(white_used_time > WHITE_TIME){
        std::cout << "Black wins on time " << std::endl;
    } else if(black_used_time > BLACK_TIME){
        std::cout << "White wins on time " << std::endl;
    }
}

void Run::run_perft(){
    board.view_board();
    std::vector<Move> moves = movegen.generate_moves();

    while(run){
        int depth = get_perft_depth();
        perftDriver(depth, moves);
    }
}

void Run::perftDriver(int& depth, const std::vector<Move>& moves){
    int num_pos = 0, total_pos = 0;
    auto start = high_resolution_clock::now();

    for(auto move : moves){
        board.make_move(move);
        num_pos = movegenTest(depth-1);
        board.undo_move();

        std::cout << move << " " << num_pos << std::endl;
        total_pos += num_pos;
    }

    auto end = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>(end-start);

    std::cout << "nodes " << std::to_string(total_pos) << std::endl;

    std::cout << "Time taken: " << std::to_string(duration.count()) << " ms" << std::endl;
    std::cout << "\n";
}

int Run::movegenTest(int depth){
    std::vector<Move> moves = movegen.generate_moves();
    
    if(depth == 0){
        return 1;
    }

    int num_nodes = 0;    

    for(auto move : moves){
        board.make_move(move);
        num_nodes += movegenTest(depth-1);                               
        board.undo_move();
    }

    return num_nodes;
}

std::string Run::get_colour_choice(){
    std::string input;
    std::cout << "Provide colour choice (w/b) " << std::endl;
    std::cout << ">>: ";
    std::cin >> input;

    while(input != "w" && input != "b"){
        std::cout << ">>: ";
        std::cin >> input;
    }

    return input;
}

void Run::get_input_from_player(){
    board.view_board();

    player_start_time = high_resolution_clock::now();
    
    std::string input;
    std::cout << ">> ";
    std::cin >> input;

    while(!std::regex_match(input, MOVE_FORMAT) && input != "undo" && input != "quit"){
        std::cout << "Inputs are undo, quit or a chess move." << std::endl;
        std::cout << "Type moves in long algebraic notation. " << std::endl;
        std::cout << "Examples:  e2e4, e7e5, e1g1 (white short castling), e7e8q (for promotion)" << std::endl;
        std::cout << ">> ";
        std::cin >> input;
    }

    if(input == "undo"){
        if(board.undo_move() == 0){movegen.generate_moves();}
    } else if(input == "quit"){
        run = false;
    } else if (std::regex_match(input, MOVE_FORMAT)){
        parse_player_move(input);
    } else {
        std::cout << "Inputs are undo, quit or a chess move in long algebraic format" << std::endl;
    }
}

int Run::get_perft_depth(){
    std::string input;
    std::cout << "Provide search depth" << std::endl;
    std::cout << ">>: ";
    std::cin >> input;

    while(!isStringDigit(input) || (input == "0")){
        std::cout << ">>: ";
        std::cin >> input;
    }

    return std::stoi(input);
}

void Run::set_engine(int& depth){
    std::string input;

    std::cout << "Engine version " << std::endl;
    std::cout << ">>: ";
    std::cin >> input;

    while(!std::regex_match(input, VERSION_FORMAT)){
        std::cout << "v0, v1, v2" << std::endl;
        std::cout << ">>: ";
        std::cin >> input;
    }

    if(input == "v0"){
        engine = std::make_unique<Enginev0>(&board, &movegen, depth);
    } else if(input == "v1"){
        engine = std::make_unique<Enginev1>(&board, &movegen, depth);
    } else if(input == "v2"){
        engine = std::make_unique<Enginev2>(&board, &movegen, depth);
    }
}

void Run::parse_player_move(std::string& str_move){
    auto move_size = str_move.size();
    std::string from(1, str_move[0]);
    from += str_move[1];

    std::string to(1,str_move[2]);
    to += str_move[3];

    std::string promo_piece = "";

    if(move_size == 5){promo_piece = str_move[4];}

    make_player_move(std::make_tuple(from, to, promo_piece));
}

void Run::make_player_move(const std::tuple<std::string, std::string, std::string>& str_move){
    Move move(0,0,0);

    auto check = convert_to_move(str_move, move);
    
    if(check != 0){
        std::cout << "There's no piece at the square chosen"<< std::endl;
    } else {
        if(movegen.move_is_legal(move)){
            player_end_time = high_resolution_clock::now();

            if(player_side){
                black_used_time += duration_cast<seconds>(player_end_time - player_start_time);
            } else {
                white_used_time += duration_cast<seconds>(player_end_time - player_start_time);
            }
            
            board.make_move(move);
            movegen.generate_moves();
        } else {
            std::cout << "Move entered is not valid " << std::endl;
            std::cout << "legals" << std::endl;
            for(auto move : movegen.get_legal_moves()){
                std::cout << move << std::endl;
            }
            get_input_from_player();
        }
    }
}

/// Convert to move turns use string input into 'Move' object. This is checked for validity by searching in generated moves 
/// If not present, the move wasn't valid
/// This function allows users to make nonsense moves, but they won't be made since they won't exist in generated moves
/// This simplifies this function which is nice
int Run::convert_to_move(const std::tuple<std::string, std::string, std::string>& str_move, Move& move){
    uint from, to, flags;

    auto [from_str, to_str, promo_piece] = str_move;

    int diff = 0;
    piece_names moving_piece;

    from = alg_to_int(from_str);
    to = alg_to_int(to_str);
    moving_piece = board.get_piece_on_square(from);

    diff = from - to;

    if(moving_piece == None){
        return -1;
    } else {
        if(promo_piece == ""){
            // no promotion

            if(board.is_occupied(to)){
                flags = 4;
            } else {
                if(moving_piece == P || moving_piece == p){
                    if(abs(diff) == 9 || abs(diff) == 7){
                        flags = 5;
                    } else if(diff == 16 || diff == -16){
                        flags = 1;
                    } else {
                        flags = 0;
                    }
                } else if(moving_piece == K || moving_piece == k){
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
            flags = (board.is_occupied(to)) ? promo_flags[promo_piece+"c"] : promo_flags[promo_piece];
        }

        move = Move(from,to,flags);
        return 0;
    }
}
