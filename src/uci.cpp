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

void Uci::uci_communication(){

    while(run){
        std::getline(std::cin, input);
        input_size = input.size();

        std::string first = get_first(input, ' ');

        if(first == "uci"){
            process_uci();
        } else if(first == "position"){
            process_position();
        } else if(first == "ucinewgame"){
            board->clear_bitboards();        
        } else if(first == "quit"){
            run = false;
        } else if(first == "isready"){
            process_isready();
        } else if(first == "debug"){
            process_debug();
        } else if(first == "print"){
            board->view_board();
        }

        pointer = 0;
        input = "";

    }
}

void Uci::process_isready(){
    // init pv table, pass size in bytes for the table
    init_pv_table(&board->pv_table, 0x400000);

    std::cout << "readyok" << std::endl;
}

void Uci::process_debug(){
    pointer += 6;

    std::string next = input.substr(pointer);

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
    pointer+=9;

    if(!input.compare(pointer, 3, "fen") || !input.compare(pointer, 8, "startpos")){
        std::string fen;
        bool check;

        if(input[pointer] == 'f'){
            pointer += 4;
            check = get_next_uci_param(input, fen, "moves", pointer);
            pointer+=fen.size();
        } else{
            check = get_next_uci_param(input, fen, "moves", pointer);
            fen = STARTING_FEN;
            pointer += 9;
        }

        fen = removeWhiteSpace(fen);

        if(check){
            board->init_from_fen(fen);  
            movegen->set_state(board); 
            movegen->generate_moves();

            if(pointer < input_size){
                pointer += 6;

                std::string _move;
                Move move;

                while(pointer < input_size){
                    _move = input.substr(pointer, 5);
                    std::tuple<std::string, std::string, std::string> str_move = parse_player_move(_move);

                    if(convert_to_move(str_move, movegen, move) == 0){
                        board->make_move(move);
                        movegen->generate_moves();
                    } else {
                        break;
                    }
                    pointer += 5;
                }
            }
        }
    }
}

