#ifndef RUN_H
#define RUN_H

#include "board.h"
#include "movegen.h"
#include "engine.h"
#include "zobrist.h"
#include "uci.h"
#include <map>

const seconds WHITE_TIME = seconds(2500);
const seconds BLACK_TIME = seconds(2500);

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

        std::unique_ptr<Engine> engine;

        colour player_side;

        seconds white_used_time = seconds(0), black_used_time = seconds(0);
        _V2::system_clock::time_point player_start_time, player_end_time;
};

#endif
