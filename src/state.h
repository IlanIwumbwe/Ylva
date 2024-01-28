#ifndef STATE_H
#define STATE_H

#include "defs.h"
#include "move.h"
#include <memory>

// Store state before a move is made that changes the state
struct State{
    uint8_t castling_rights;  
    int hm_clock;
    //std::vector<Move> valid_moves;
    piece_names recent_capture;
    Move prev_move; 
    std::shared_ptr<State> prev_state = NULL;

    static int state_id;

    State(uint8_t cr, int hmc, piece_names rcap, Move prev){
        castling_rights = cr;
        hm_clock = hmc;
        recent_capture = rcap;
        prev_move = prev;

        state_id++;
    }
};

int State::state_id = 0;

#endif
