#ifndef LOOP_H
#define LOOP_H

#include "board.h"

class Loop{
    public:
        Loop(std::string& fen, game_modes mode) : board(fen), mode(mode) {}
    
        void run_game_loop(){
            while(run){
                board.view_board();

                if(mode == PVP){
                    get_input_from_player();
                } else if(mode == PVE){
                    if(board.get_turn() == WHITE){get_input_from_player();}
                    else {
                        //TODO make_engine_move(); 
                        std::cout << "Engine move; " << std::endl;
                    }
                } else if(mode == EVE){
                    //TODO make_engine_move(); 
                    std::cout << "Engine move; " << std::endl;
                } else {
                    std::cerr << "Unexpected mode " << mode << std::endl;
                }
            }
        }

        void get_input_from_player(){
            std::string input;
            std::cout << ">> ";
            std::cin >> input;

            std::regex pattern(MOVE_FORMAT);    

            while(!std::regex_match(input, pattern) && input != "undo" && input != "quit"){
                std::cout << "Inputs are undo, quit or a chess move." << std::endl;
                std::cout << "Type moves in long algebraic notation. " << std::endl;
                std::cout << "Examples:  e2e4, e7e5, e1g1 (white short castling), e7e8q (for promotion)" << std::endl;
                std::cout << ">> ";
                std::cin >> input;
            }

            if(input == "undo"){
                board.undo_move();
            } else if(input == "quit"){
                run = false;
            } else if (std::regex_match(input, pattern)){
                parse_player_move(input);
            } else {
                std::cout << "Inputs are undo, quit or a chess move in long algebraic format" << std::endl;
            }
        }

        void parse_player_move(std::string& str_move){
            std::tuple<std::string, std::string, std::string> move;

            auto move_size = str_move.size();
            std::string from(1, str_move[0]);
            from += str_move[1];

            std::string to(1,str_move[2]);
            to += str_move[3];

            std::string promo_piece = "";

            if(move_size == 5){promo_piece = str_move[4];}

            move = std::make_tuple(from, to, promo_piece);

            make_player_move(move);
        }

        bool move_is_valid(const Move& move){
            //TODO
            return true;
        }

        void make_player_move(std::tuple<std::string, std::string, std::string>& str_move){
            auto move = convert_to_move(str_move);
            
            while(!move_is_valid(move)){
                std::cout << "Move entered is not a valid chess move " << std::endl;
                get_input_from_player();
                auto move = convert_to_move(str_move);
            }

            board.make_move(move);   
            board.change_turn();
        }

        /// Convert to move turns use string input into 'Move' object. This is checked for validity by searching in generated moves 
        /// If not present, the move wasn't valid
        /// This function allows users to make nonsense moves, but they won't be made since they won't exist in generated moves
        /// This simplifies this function which is nice
        Move convert_to_move(std::tuple<std::string, std::string, std::string>& str_move){
            std::string from_str, to_str, promo_piece;
            unsigned int from, to, flags;

            std::tie(from_str, to_str, promo_piece) = str_move;

            int diff = 0;
            piece_names moving_piece;

            from = alg_to_int(from_str);
            to = alg_to_int(to_str);
            diff = from - to;

            moving_piece = board.get_piece_on_square(from);

            if(promo_piece == ""){
                // no promotion

                if(board.is_square_occupied(to)){
                    flags = 4;
                } else {
                    if(moving_piece == P || moving_piece == p){
                        if(abs(diff) == 9 || abs(diff) == 7){
                            flags = 5;
                        } else if(diff == 2 || diff == -2){
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
                flags = (board.is_square_occupied(to)) ? promo_flags[promo_piece+"c"] : promo_flags[promo_piece];
            }

            return Move(from,to,flags);
        }

    private:
        Board board;
        game_modes mode;
        bool run = true;
};




#endif