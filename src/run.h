#ifndef RUN_H
#define RUN_H

#include "board.h"
#include "movegen.h"
#include "engine.h"
#include "zobrist.h"
#include "uci.h"
#include <map>

class Run{
    public:
        Run(std::string& fen, game_modes mode);

        void run_PVP();

        void run_PVE();

        void run_PVE_UCI();

        void run_EVE();

        bool end_game();

        void get_input_from_player();

        std::string get_colour_choice();

        int get_perft_depth();

        void set_engine(int& depth);

        void make_player_move(const std::tuple<std::string, std::string, std::string>& str_move);
        
    private:
        Board board;
        MoveGen movegen;
        game_modes mode;
        bool run = true;

        std::shared_ptr<Engine> engine;

        colour player_side;
};

#endif
