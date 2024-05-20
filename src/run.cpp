#include "run.h"

Run::Run(std::string& fen, game_modes mode) : mode(mode) {
    if(mode == PVP){
        board.init_from_fen(fen);  
        movegen.set_state(&board);
        movegen.generate_moves();   

        run_PVP();

    } else if(mode == PVE){
        board.init_from_fen(fen);  
        // init pv table, pass size in bytes for the table
        init_pv_table(&board.pv_table, 0x400000);
        movegen.set_state(&board);    
        movegen.generate_moves();   

        int depth = get_perft_depth();
        set_engine(depth);
        run_PVE();

    } else if(mode == EVE) {
        board.init_from_fen(fen);  
        // init pv table, pass size in bytes for the table
        init_pv_table(&board.pv_table, 0x400000);
        movegen.set_state(&board); 
        movegen.generate_moves();      

        int depth = get_perft_depth();
        set_engine(depth);
        run_EVE();

    } else if(mode == UCI){
        run_PVE_UCI();
        
    } else {
        std::cerr << "Unexpected mode " << mode << std::endl;
    }
}

void Run::run_PVP(){
    while (!end_game()){
        get_input_from_player();
    }
}

void Run::run_PVE(){
    std::string colour_choice = get_colour_choice();

    player_side = (colour_choice == "w") ? WHITE : BLACK;

    while(!end_game()){
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
    }
}

void Run::run_PVE_UCI(){
    Uci uci(&board, &movegen);
    uci.uci_communication();
}

void Run::run_EVE(){
    while(!end_game()){
        engine->engine_driver();
    }
}

bool Run::end_game(){
    if(board.get_hm_clock() == 100){
        std::cout << "Draw by 50 move rule" << std::endl;
        board.view_board();
    } else if(movegen.no_legal_moves()){
        board.view_board();
        if(movegen.ally_king_in_check()){
            std::cout << (board.get_turn() ? "White " : "Black ") << "wins by checkmate" << std::endl;
        } else { std::cout << "Draw by stalemate" << std::endl; }
    } else if(white_used_time > WHITE_TIME){
        std::cout << "Black wins on time " << std::endl;
    } else if(black_used_time > BLACK_TIME){
        std::cout << "White wins on time " << std::endl;
    } else {return !run;} 

    return true;
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
        std::tuple<std::string, std::string, std::string> str_move = parse_player_move(input);
        make_player_move(str_move);
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

void Run::make_player_move(const std::tuple<std::string, std::string, std::string>& str_move){
    Move move(0,0,0);

    auto check = convert_to_move(str_move, &movegen, move);

    if(check == 0){
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

