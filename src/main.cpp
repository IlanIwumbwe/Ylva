#include <iostream>
#include <unistd.h>
#include <cstring>
#include "zobrist.h"
#include "uci.h"

int main(){
    populate_attack_sets();
    populate_rays();
    init_hash_keys();

    Board board;
    MoveGen movegen;
    Uci uci(&board, &movegen);
    uci.uci_communication();

    return 0;
}

