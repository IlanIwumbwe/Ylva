#ifndef MOVE_H
#define MOVE_H

#include "defs.h"
class Move{
    public:
        Move(){
            move = 0;
        }

        Move(uint from, uint to, uint flags){
            move = ((flags & 0xf) << 12) | ((from & 0x3f) << 6) | (to & 0x3f);
        }

        uint get_to() const {return move & 0x3f;}
        uint get_from() const {return (move >> 6) & 0x3f;}
        uint get_flags() const {return (move >> 12) & 0xf;}
        uint get_move() const {return move;}

        void set_to(uint to) {move &= ~0x3f; move |= to & 0x3f;}
        void set_from(uint from) {move &= ~0xfc0; move |= (from & 0x3f) << 6;}
      
        bool is_promo() const {return (move & PROMO_FLAG) != 0;}
        bool is_capture() const {return (move & CAPTURE_FLAG) != 0;}

        bool operator==(Move a) const {return move == a.get_move();}
        bool operator!=(Move a) const {return move != a.get_move();}

        friend std::ostream& operator<<(std::ostream& os, const Move& move){
            os << int_to_alg(move.get_from()) << int_to_alg(move.get_to());
            auto f = move.get_flags();

            if(f == 8 || f == 12){os << "n";}
            else if(f == 9 || f == 13){os << "b";}
            else if(f == 10 || f == 14){os << "r";}
            else if(f == 11 || f == 15){os << "q";}

            return os;
        }

    private:
        uint16_t move;

};

#endif
