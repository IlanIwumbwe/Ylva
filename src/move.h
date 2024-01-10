#ifndef MOVE_H
#define MOVE_H

#include "helper_funcs.h"
#include "defs.h"


class Move{
    public:
        Move(unsigned int from, unsigned int to, unsigned int flags){
            move = ((flags & 0xf) << 12) | ((from & 0x3f) << 6) | (to & 0x3f);
        }

        unsigned int get_to() const {return move & 0x3f;}
        unsigned int get_from() const {return (move >> 6) & 0x3f;}
        unsigned int get_flags() const {return (move >> 12) & 0x0f;}
        u_int16_t get_move() const {return move;}

        void set_to(unsigned int to) {move &= ~0x3f; move |= to & 0x3f;}
        void set_from(unsigned int from) {move &= ~0xfc0; move |= (from & 0x3f) << 6;}
      
        bool is_capture() const {return (move & CAPTURE_FLAG) != 0;}
        bool is_promo() const {return (move & PROMO_FLAG) != 0;}

        bool operator==(Move a) const {return move == a.get_move();}
        bool operator!=(Move a) const {return move != a.get_move();}

        friend std::ostream& operator<<(std::ostream& os, Move& move) {
            return os << "from " << move.get_from() << " to " << move.get_to() << " flags " << move.get_flags() << std::endl;
        }

    private:
        uint16_t move;

};

#endif
