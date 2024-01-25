#include <iostream>
#include <unistd.h>
#include "run.h"

int main(int argc, char* argv[]){
    std::string fen;
    game_modes mode = PERFT;

    int opt;

    while ((opt = getopt(argc, argv, "f:m:")) != -1) {
        switch (opt) {
            case 'f':
                fen = optarg;
                break;
            case 'm':
                if(strcmp(optarg,"pvp") == 0){
                    mode = PVP;
                } else if(strcmp(optarg,"pve") == 0){
                    mode = PVE;
                } else if(strcmp(optarg,"eve") == 0){
                    mode = EVE;
                } else {
                    mode = PERFT;
                }
            default:
                break;
        }
    }

    if (fen == "" && opt == 'f') {
        std::cerr << "Usage: " << argv[0] << " -f <fen_string>" << std::endl;
        std::cerr << "Missing required arguments." << std::endl;
        return 1;
    }

    populate_attack_sets();
    populate_rays();

    Run game_loop(fen, mode);

    return 0;
}



