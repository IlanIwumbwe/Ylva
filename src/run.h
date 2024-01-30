#ifndef RUN_H
#define RUN_H

#include "board.h"
#include "movegen.h"
#include <chrono>
using namespace std::chrono;

class Run{
    public:
        Run(std::string& fen, game_modes mode) : board(fen), movegen(&board), mode(mode) {
            // generate moves for the start state (no moves made yet)
            movegen.generate_moves();

            if(mode == PVP){
                run_PVP();
            } else if(mode == PVE){
                run_PVE();
            } else if(mode == EVE) {
                run_EVE();
            } else if(mode == PERFT){
                run_perft();

            } else {
                std::cerr << "Unexpected mode " << mode << std::endl;
            }
        }

        void run_PVP(){
            while (run){
                get_input_from_player();
            }
        }

        void run_PVE(){
            while(run){
                if(board.get_turn() == WHITE){
                    get_input_from_player();
                }else{
                    /// TODO: make_engine_move();
                }
            }
        }

        void run_EVE(){
            while(run){
                /// TODO: make_engine_move();
            }
        }

        void run_perft(){
            auto moves = board.get_valid_moves();

            while(run){
                board.view_board();

                int depth = get_perft_depth();
                int num_pos = 0, total_pos = 0;

                auto start = high_resolution_clock::now();

                for(auto move : moves){
                    make_move(move);
                    num_pos = movegenTest(depth-1);
                    board.undo_move();

                    std::cout << move << ": " << num_pos << std::endl;
                    total_pos += num_pos;
                }

                auto end = high_resolution_clock::now();

                auto duration = duration_cast<milliseconds>(end-start);
                
                std::cout << "Time taken: " << std::to_string(duration.count()) << " ms" << std::endl;
                std::cout << "Total positions: " << std::to_string(total_pos) << std::endl;
                std::cout << "\n";
            }
        }

        int movegenTest(int depth){
            auto moves = board.get_valid_moves();
            
            if(depth == 0){
                return 1;
            }

            int num_nodes = 0;    

            for(auto move : moves){
                make_move(move);
                num_nodes += movegenTest(depth-1);                               
                board.undo_move();
                movegen.generate_moves();
                // if(m.get_from() == 48 && m.get_to() == 32){std::cout << "move: " << move << " " << move.get_flags() << std::endl;}
            }

            return num_nodes;
        }

        // whenever a move is made on the board, generate new moves for the new state
        void make_move(const Move& move){
            board.make_move(move);
            movegen.generate_moves();
        }

        void get_input_from_player(){
            board.view_board();

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
                if(board.undo_move() == 0){movegen.generate_moves();};
            } else if(input == "quit"){
                run = false;
            } else if (std::regex_match(input, MOVE_FORMAT)){
                parse_player_move(input);
            } else {
                std::cout << "Inputs are undo, quit or a chess move in long algebraic format" << std::endl;
            }
        }

        int get_perft_depth(){
            std::string input;
            std::cout << ">>: ";
            std::cin >> input;

            while(!isStringDigit(input)){
                std::cout << ">>: ";
                std::cin >> input;
            }

            return std::stoi(input);
        }

        void parse_player_move(std::string& str_move){
            auto move_size = str_move.size();
            std::string from(1, str_move[0]);
            from += str_move[1];

            std::string to(1,str_move[2]);
            to += str_move[3];

            std::string promo_piece = "";

            if(move_size == 5){promo_piece = str_move[4];}

            make_player_move(std::make_tuple(from, to, promo_piece));
        }

        void make_player_move(const std::tuple<std::string, std::string, std::string>& str_move){
            Move move(0,0,0);

            auto check = convert_to_move(str_move, move);
            
            if(check != 0){
                std::cout << "There's no piece at the square chosen"<< std::endl;
            } else {
                if(board.move_is_valid(move)){
                    make_move(move);
                } else {
                    std::cout << "Move entered is not valid " << std::endl;
                    get_input_from_player();
                }
            }
        }

        /// Convert to move turns use string input into 'Move' object. This is checked for validity by searching in generated moves 
        /// If not present, the move wasn't valid
        /// This function allows users to make nonsense moves, but they won't be made since they won't exist in generated moves
        /// This simplifies this function which is nice
        int convert_to_move(const std::tuple<std::string, std::string, std::string>& str_move, Move& move){
            uint from, to, flags;

            auto [from_str, to_str, promo_piece] = str_move;

            int diff = 0;
            piece_names moving_piece;

            from = alg_to_int(from_str);
            to = alg_to_int(to_str);
            diff = from - to;

            moving_piece = board.get_piece_on_square(from);

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

    private:
        Board board;
        MoveGen movegen;
        game_modes mode;
        bool run = true;
};

#endif
