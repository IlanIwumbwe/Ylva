#include <iostream>
#include <unistd.h>
#include "loop.h"

int main(int argc, char* argv[]){
    std::string fen;

    int opt;

    while ((opt = getopt(argc, argv, "f:")) != -1) {
        switch (opt) {
            case 'f':
                fen = optarg;
                break;
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

    Loop game_loop(fen, PVP);

    return 0;
}



