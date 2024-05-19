#ifndef UCI_H
#define UCI_H

#include "defs.h"
#include "board.h"
#include "movegen.h"
#include "move.h"

int convert_to_move(const std::tuple<std::string, std::string, std::string>& str_move, MoveGen* movegen, Move& move);
std::tuple<std::string, std::string, std::string> parse_player_move(std::string& str_move);

class Uci{
    public:
        Uci(Board* _board, MoveGen* _movegen) : board(_board), movegen(_movegen) {}
        void uci_communication();
        void process_uci();
        void process_position();
        void process_isready();
        void process_debug();

    private:
        std::string input;
        size_t pointer = 0;
        Board* board;
        MoveGen* movegen;

        size_t input_size = 0;

        bool run = true;
};

#endif