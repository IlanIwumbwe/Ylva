#include <iostream>
#include <unistd.h>
#include <cstring>
#include "run.h"
#include "zobrist.h"

int main(int argc, char* argv[]){
    std::string fen = "";
    game_modes mode = UCI;
    int opt;

    while ((opt = getopt(argc, argv, "f:m:")) != -1) {
        switch (opt) {
            case 'f':
                fen = optarg;
                break;
            case 'm':
                if(strcmp(optarg,"pvp") == 0){
                    mode = PVP;
                } else if(strcmp(optarg,"uci") == 0){
                    mode = UCI;
                }
                break;
            default:
                break;
        }
    }

    if (fen == "" && opt == 'f') {
        std::cerr << "Usage: " << argv[0] << " -f <fen_string> -m <MODE>" << std::endl;
        std::cerr << "Missing required arguments." << std::endl;
        return 1;
    }

    populate_attack_sets();
    populate_rays();
    init_hash_keys();

    if(fen == ""){
        fen = STARTING_FEN;
    }

    Run game_loop(fen, mode);

    return 0;
}



