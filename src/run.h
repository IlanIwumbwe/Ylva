#ifndef RUN_H
#define RUN_H

#include "board.h"
#include "movegen.h"
#include "engine.h"
#include <map>

class Run{
    public:
        Run(std::string& fen, game_modes mode);

        void run_PVP();

        void run_PVE();

        void run_EVE();

        void end_game();

        void run_perft();

        void perftDriver(int& depth, std::vector<Move> moves);

        int movegenTest(int depth);

        // whenever a move is made on the board, generate new moves for the new state
        void make_move(const Move& move);

        void get_input_from_player();

        int get_perft_depth();

        void parse_player_move(std::string& str_move);

        void make_player_move(const std::tuple<std::string, std::string, std::string>& str_move);

        /// Convert to move turns use string input into 'Move' object. This is checked for validity by searching in generated moves 
        /// If not present, the move wasn't valid
        /// This function allows users to make nonsense moves, but they won't be made since they won't exist in generated moves
        /// This simplifies this function which is nice
        int convert_to_move(const std::tuple<std::string, std::string, std::string>& str_move, Move& move);
        
    private:
        Board board;
        MoveGen movegen;
        game_modes mode;
        bool run = true;
        Enginev1 engine_v1;
};

#endif
