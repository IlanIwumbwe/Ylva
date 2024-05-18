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

    if(promo_piece == ""){
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

std::tuple<std::string, std::string, std::string> parse_player_move(std::string& str_move){
    auto move_size = str_move.size();
    std::string from(1, str_move[0]);
    from += str_move[1];

    std::string to(1,str_move[2]);
    to += str_move[3];

    std::string promo_piece = "";

    if(move_size == 5){promo_piece = str_move[4];}

    return std::make_tuple(from, to, promo_piece);
}

void Uci::uci_communication(){

    while(run){
        std::getline(std::cin, input);

        std::string first = get_first(input, ' ');

        if(first == "uci"){
            process_uci();
        } else if(first == "position"){
            process_position();
        } else if(first == "ucinewgame"){
            board->clear_bitboards();        
        } else if(first == "quit"){
            run = false;
        }

        pointer = 0;
        input = "";

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

    if(!input.compare(pointer, 3, "fen")){
        pointer += 3;
        std::string fen;
        bool check = get_next_uci_param(input, fen, "moves", pointer);
        pointer+=fen.size();
        fen = removeWhiteSpace(fen);

        if(check){
            board->init_from_fen((fen == "startpos") ? STARTING_FEN : fen);  
            movegen->set_state(board); 
            movegen->generate_moves();

            if(pointer != input.size()){
                pointer += 6;

                std::vector<Move> moves;
                std::string _move;
                Move move;

                while(pointer < input.size()){
                    _move = input.substr(pointer, 5);
                    std::tuple<std::string, std::string, std::string> str_move = parse_player_move(_move);

                    if(convert_to_move(str_move, movegen, move) == 0){
                        moves.push_back(move);
                    } else {
                        moves.clear();
                        break;
                    }
                    pointer += 5;
                }

                for(Move move : moves){
                    board->make_move(move);
                    movegen->generate_moves();
                }
            }
        }
    }

    board->view_board();
}

