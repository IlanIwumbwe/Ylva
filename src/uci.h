#ifndef UCI_H
#define UCI_H

#include "defs.h"
#include "board.h"
#include "movegen.h"
#include "move.h"
#include "engine.h"

int convert_to_move(const std::tuple<std::string, std::string, std::string>& str_move, MoveGen* movegen, Move& move);
std::tuple<std::string, std::string, std::string> parse_player_move(std::string& str_move);
bool is_valid_go_param(std::string token);

class Uci{
    public:
        Uci(Board* _board, MoveGen* _movegen){
            board = _board;
            movegen = _movegen;
        }

        void uci_communication();

        std::string current_token(){
            if(pointer < tokens.size()){
                return tokens[pointer];
            } else {
                return "";
            }
        }

        std::string previous_token(){
            assert((0 < pointer) && (pointer < tokens.size()));
            return tokens[pointer-1];
        }

        void perft_driver(int& depth);

        int movegen_test(int depth);

        bool out_of_tokens(){return pointer == tokens.size()-1;}
        void process_uci();
        void process_position();
        void process_isready();
        void process_debug();
        void process_go();

    private:
        std::string input;
        size_t pointer = 0;
        std::vector<std::string> tokens;
        std::shared_ptr<Engine> engine;
        Board* board;
        MoveGen* movegen;

        size_t input_size = 0;
        std::vector<Move> moves_to_search;
        std::string token_s_arg;
        int token_i_arg;
        Move _move;

        std::string engine_name = "YLVA v1";

        bool run = true;
        colour engine_side;
};

#endif