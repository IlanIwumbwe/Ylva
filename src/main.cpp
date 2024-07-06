#include <iostream>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include "zobrist.h"
#include "uci.h"

int main(){
    populate_attack_sets();
    populate_rays();
    init_hash_keys();
    
    /*
    U64 magic;

    std::cout << "{";
    // generate magics for bishop
    for(int square = 0; square < 64; square++){
        magic = find_magic(square, BBits[square], true);

        while(magic == 0ULL){
            magic = find_magic(square, BBits[square], true);
        }

        std::cout << magic << ", ";
    }
    std::cout << "}\n";

    std::cout << "{";
    

    // generate magics for rook
    for(int square = 0; square < 64; square++){
        magic = find_magic(square, RBits[square], false);

        while(magic == 0ULL){
            magic = find_magic(square, RBits[square], false);
        }

        std::cout << magic << ", ";
    }
    std::cout << "}";
    */

    Board board;
    MoveGen movegen;
    Uci uci(&board, &movegen);
    uci.uci_communication();

    return 0;
}

