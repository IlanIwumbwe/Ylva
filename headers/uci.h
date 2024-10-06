#ifndef UCI_H
#define UCI_H

#include "../headers/movegen.h"
#include "../headers/board.h"

#define INPUT_SIZE 8192
#define UCI_TOKENS_SIZE 500

enum {
    NONE = 0,
    UCI = 127,
    GO = 11,
    UCINEWGAME = 6,
    POSITION = 17,
    STOP = 28,

#ifdef DEV
    PERFT = 116,
    PRINT = 112,
#endif

};

void uci_communication();

#endif