#ifndef BOARD_H
#define BOARD_H

#include "defs.h"
#include "move.h"
#include "helper_funcs.h"
#include <assert.h>

class Board{
    public:
        Board (const std::string& _fen){
            // parse FEN string
            std::vector<std::string>parts = splitString((_fen == "") ? STARTING_FEN : removeWhiteSpace(_fen), ' ');
            
            init_from_fen(parts);
        }

        /// Make move given as input on the board
        void make_move(const Move& move){
            const auto from = move.get_from();
            const auto to = move.get_to();
            const auto flags = move.get_flags();

            const auto from_piece_name = get_piece_on_square(from);
            const auto to_piece_name = get_piece_on_square(to);

            auto from_piece_colour = get_piece_colour(from_piece_name);

            //assert(from_piece_name != None);

            U64 from_piece_bitboard = get_piece_bitboard(from_piece_name);

            piece_names promo_piece_name;
            U64 promotion_piece_bitboard;

            castling_rights_history.push_back(castling_rights);

            // remove castling rights if king or rook moves
            if(from_piece_name == K){
                castling_rights &= ~(K_castle | Q_castle);
            } else if(from_piece_name == k){
                castling_rights &= ~(k_castle | q_castle);
            } else if(from_piece_name == R && from == 0){
                castling_rights &= ~K_castle;
            } else if(from_piece_name == R && from == 7){
                castling_rights &= ~Q_castle;
            } else if(from_piece_name == r && from == 56){
                castling_rights &= ~k_castle;
            } else if (from_piece_name == r && from == 63){
                castling_rights &= ~q_castle;
            }

            if(!move.is_promo()){
                // place piece at to square if not promotion move
                from_piece_bitboard |= set_bit(to);
                
                if(flags == 4){
                    capture_piece(to_piece_name, set_bit(to));
                } else if(flags == 5){
                    ep_capture(from_piece_colour, to);
                } else if(flags == 2){
                    castle_kingside(from, from_piece_colour);
                } else if(flags == 3){
                    castle_queenside(from, from_piece_colour);
                } else {
                    // quiet moves and double pawn pushes
                    if(from_piece_name == P || from_piece_name == p){hm_clock_reset_history.push_back(hm_clock); hm_clock = 0;} // pawn advance
                    else{hm_clock++;}  // other quiet moves
                }
            
            } else {
                // set the name of to_piece to that of the piece we want to promote to
                // set the bitboard of the piece that's been promoted to

                if(move.is_capture()){
                    // move is a promotion with capture move
                    capture_piece(to_piece_name, set_bit(to));
                } else {hm_clock = 0;} 

                promo_piece_name = get_promo_piece(flags, from_piece_colour); // piece that we want to promote to
                promotion_piece_bitboard = get_piece_bitboard(promo_piece_name);
                promotion_piece_bitboard |= set_bit(to);
                set_piece_bitboard(promo_piece_name, promotion_piece_bitboard);
            }

            // remove piece from initial square in its bitboard, then set the bitboard
            from_piece_bitboard &= ~set_bit(from);
            set_piece_bitboard(from_piece_name, from_piece_bitboard);

            // add move to move history
            move_history.push_back(move);

            change_turn();
        }

        int get_prev_move(Move& prev_move){
            if(move_history.size() != 0){
                prev_move = move_history.back();
                return 0;
            } else {
                return -1;
            }
        }

        int undo_move(){
            if(move_history.size() != 0){  
                auto prev_move = move_history.back();
                move_history.pop_back();

                const auto from = prev_move.get_from();
                const auto to = prev_move.get_to();
                const auto flags = prev_move.get_flags();

                piece_names promo_piece_name;
                U64 promotion_piece_bitboard;

                piece_names from_piece_name;
                colour from_piece_colour;
                U64 from_piece_bitboard;

                if(!prev_move.is_promo()){
                    // remove piece from to square on its bitboard if not a promotion move
                    from_piece_name = get_piece_on_square(to);
                    from_piece_bitboard = get_piece_bitboard(from_piece_name) & ~set_bit(to);
                    from_piece_colour = get_piece_colour(from_piece_name);

                    if(flags == 4){
                        uncapture_piece(set_bit(to));
                    } else if(flags == 5){
                        ep_uncapture(from_piece_colour, to);  
                    } else if(flags == 2){
                        uncastle_kingside(from, from_piece_colour); 
                    } else if(flags == 3){
                        uncastle_queenside(from, from_piece_colour); 
                    } else {
                        // quiet moves and double pawn pushes
                        if(from_piece_name == P || from_piece_name == p){hm_clock = hm_clock_reset_history.back(); hm_clock_reset_history.pop_back();} // pawn advance
                        else{hm_clock--;}  // other quiet moves
                    }

                } else {
                    // remove piece that was promoted to
                    promo_piece_name = get_piece_on_square(to); // piece that we wanted to promote to
                    promotion_piece_bitboard = get_piece_bitboard(promo_piece_name) & ~set_bit(to);
                    set_piece_bitboard(promo_piece_name, promotion_piece_bitboard);

                    if(prev_move.is_capture()){
                        uncapture_piece(set_bit(to));
                    } else {
                        hm_clock = hm_clock_reset_history.back();
                        hm_clock_reset_history.pop_back();
                    }
                
                    // get correct pawn bitboard
                    from_piece_name = get_piece_colour(promo_piece_name) ? p : P;
                    from_piece_bitboard = get_piece_bitboard(from_piece_name);

                    //std::cout << "from: " << name_to_char(from_piece_name) << std::endl;
                    //std::cout << "promo: " << name_to_char(promo_piece_name) << std::endl;
                    //printbitboard(promotion_piece_bitboard); 
                }

                // put piece at initial square in its bitboard, then set the bitboard
                from_piece_bitboard |= set_bit(from);
                set_piece_bitboard(from_piece_name, from_piece_bitboard);

                change_turn();

                castling_rights = castling_rights_history.back();
                castling_rights_history.pop_back();

                return 0;
            } else {
                std::cout << "Cannot undo, no move has been made yet" << std::endl;
                return -1;
            }
        } 

        /// Perform en-passant capture
        void ep_capture(const colour& pawn_colour, const unsigned int& to){
            U64 captured_piece_bitboard = 0;
            piece_names captured_piece_name;
            unsigned int captured_piece_square;

            if(pawn_colour == WHITE){
                captured_piece_name = p;
                captured_piece_square = to-8;
            }else{
                captured_piece_name = P;
                captured_piece_square = to+8;
            }

            captured_piece_bitboard = get_piece_bitboard(captured_piece_name);
            captured_piece_bitboard &= ~set_bit(captured_piece_square);
            set_piece_bitboard(captured_piece_name, captured_piece_bitboard);

            captured_pieces.push_back(captured_piece_name);
            hm_clock_reset_history.push_back(hm_clock);
            hm_clock = 0;
        }

        /// Revert en-passant capture
        void ep_uncapture(const colour& pawn_colour, const unsigned int& to){
            U64 captured_piece_bitboard = 0;
            piece_names captured_piece_name;
            unsigned int captured_piece_square;

            if(pawn_colour == WHITE){
                captured_piece_name = p;
                captured_piece_square = to-8;
            }else{
                captured_piece_name = P;
                captured_piece_square = to+8;
            }

            captured_piece_bitboard = get_piece_bitboard(captured_piece_name);
            captured_piece_bitboard |= set_bit(captured_piece_square);
            set_piece_bitboard(captured_piece_name, captured_piece_bitboard);

            captured_pieces.pop_back();
            hm_clock = hm_clock_reset_history.back();
            hm_clock_reset_history.pop_back();
        }

        void castle_kingside(const unsigned int& king_square, const colour& king_colour){
            auto rook_type = (king_colour) ? r : R;
            U64 rook_bitboard = get_piece_bitboard(rook_type);

            // remove rook from initial square, put at new square
            rook_bitboard &= ~set_bit(king_square - 3);
            rook_bitboard |= set_bit(king_square - 1);
            set_piece_bitboard(rook_type, rook_bitboard); 
            hm_clock++;
        }

        void uncastle_kingside(const unsigned int& king_square, const colour& king_colour){
            auto rook_type = (king_colour) ? r : R;
            U64 rook_bitboard = get_piece_bitboard(rook_type);

            // remove rook from new square, put at initial square
            rook_bitboard |= set_bit(king_square - 3);
            rook_bitboard &= ~set_bit(king_square - 1);
            set_piece_bitboard(rook_type, rook_bitboard); 
            hm_clock--;
        }

        void castle_queenside(const unsigned int& king_square, const colour& king_colour){
            auto rook_type = (king_colour) ? r : R;
            U64 rook_bitboard = get_piece_bitboard(rook_type);

            // remove rook from initial square, put at new square
            rook_bitboard &= ~set_bit(king_square + 4);
            rook_bitboard |= set_bit(king_square + 1);
            set_piece_bitboard(rook_type, rook_bitboard); 
            hm_clock++;
        }   

        void uncastle_queenside(const unsigned int& king_square, const colour& king_colour){
            auto rook_type = (king_colour) ? r : R;
            U64 rook_bitboard = get_piece_bitboard(rook_type);

            // remove rook from initial square, put at new square
            rook_bitboard |= set_bit(king_square + 4);
            rook_bitboard &= ~set_bit(king_square + 1);
            set_piece_bitboard(rook_type, rook_bitboard); 
            hm_clock--;
        }   
        
        /// Perform normal capture
        void capture_piece(const piece_names& to_piece_name, const U64& square_bitboard){
            auto captured_piece_bitboard = get_piece_bitboard(to_piece_name);
            captured_piece_bitboard &= ~square_bitboard;
            set_piece_bitboard(to_piece_name, captured_piece_bitboard);

            captured_pieces.push_back(to_piece_name);
            hm_clock_reset_history.push_back(hm_clock);
            hm_clock = 0;
        }

        /// Revert normal capture
        void uncapture_piece(const U64& square_bitboard){
            auto to_piece_name = captured_pieces.back();
            auto captured_piece_bitboard = get_piece_bitboard(to_piece_name);
            captured_piece_bitboard |= square_bitboard;
            set_piece_bitboard(to_piece_name, captured_piece_bitboard);

            captured_pieces.pop_back();
            hm_clock = hm_clock_reset_history.back();   
            hm_clock_reset_history.pop_back();
        }

        bool has_castling_rights(int flag) const {
            return (castling_rights & flag) != 0;
        }

        bool is_occupied(unsigned int square){
            return get_bit(get_entire_bitboard(), square);
        }

        piece_names get_promo_piece(const unsigned int& flags, const colour& from_piece_colour){
            if(flags == 12 || flags == 8){
                return (from_piece_colour == WHITE) ? N : n;
            } else if(flags == 13 || flags == 9){
                return (from_piece_colour == WHITE) ? B : b;
            } else if(flags == 14 || flags == 10){
                return (from_piece_colour == WHITE) ? R : r;
            } else if(flags == 15 || flags == 11){
                return (from_piece_colour == WHITE) ? Q : q;
            } else {
                std::cerr << "Unexpected flag " << flags << " for promotion type move" << std::endl;
                exit(0);
            }
        }

        static piece_names char_to_name(const char& c) {
            auto it = std::find_if(namecharint.begin(), namecharint.end(), [c](const std::pair<char, piece_names>& p) {
                return p.first == c;
            });

            if(it == namecharint.end()){
                std::cout << "What? The piece name (char) " << c << " does not exist" << std::endl;
                exit(0);
            } else {
                return it->second;
            }
        }

        static char name_to_char(const piece_names& name){
            auto it = std::find_if(namecharint.begin(), namecharint.end(), [name](const std::pair<char, piece_names>& p) {
                return p.second == name;
            });

            if(it == namecharint.end()){
                std::cout << "What? The piece name (piece_name) " << name << " does not exist" << std::endl;
                exit(0);
            } else {
                return it->first;
            }
        }

        void init_from_fen(const std::vector<std::string>& parts){
            if(parts.size() != 6){
                std::cerr << "Fen string format incorrect" << std::endl;
                exit(0);
            } else {
                init_board_state(parts[0]);
                init_turn(parts[1]);
                init_castling_rights(parts[2]);
                init_enpassant_square(parts[3]);
                init_halfmove_clock(parts[4]);
                init_fullmoves(parts[5]);
            }
        }

        /// Initialise all bitboards and piece lists
        void init_board_state(const std::string& board_string){
            int pointer = 0, current_square = 63;
            char c;

            while(current_square >= 0){
                c = board_string[pointer];
        
                if(isalpha(c)){
                    piece_names piece_name = char_to_name(c);

                    set_piece_bitboard(piece_name, get_piece_bitboard(piece_name) | set_bit(current_square));

                    current_square--;
 
                } else if(isdigit(c)){
                    current_square -= (c - '0');
                } 

                pointer++;
            }

        }
        
        void init_turn(const std::string& str_turn){
            if(str_turn == "w"){
                turn = WHITE;
            } else {
                turn = BLACK;
            }
        }

        void init_castling_rights(const std::string& str_castling){
            if(str_castling == "-"){
                castling_rights = 0;
            } else {
                for(auto c : str_castling){
                    if(c == 'K'){castling_rights |= 8;}
                    else if(c == 'Q'){castling_rights |= 4;}
                    else if(c == 'k'){castling_rights |= 2;}
                    else if(c == 'q'){castling_rights |= 1;}
                    else{std::cout << "What? Castling availability flag in fen string " << str_castling << " is unexpected" << std::endl; exit(0);}
                }
            }
        }

        /// Given the enpassant square, add the relevant move to move history that would've led to that enpassant square, which will be used to 
        /// validate possible enpassant captures by an enemy piece
        void init_enpassant_square(const std::string& str_ep_square){
            if(str_ep_square == "-"){
                ep_square = 0;
            } else {
                ep_square = alg_to_int(str_ep_square);

                if(turn == WHITE){
                    move_history.push_back(Move(ep_square+8,ep_square-8,1));
                } else {
                    move_history.push_back(Move(ep_square-8,ep_square+8,1));
                }
            }
        }

        void init_halfmove_clock(const std::string& str_hm_clock){
            hm_clock = std::stoi(str_hm_clock);
        }

        void init_fullmoves(const std::string& str_fullmoves){
            fullmoves = std::stoi(str_fullmoves);
        }

        int get_fullmoves(){
            return floor(move_history.size() / 2) + fullmoves;
        }

        colour get_turn(){return turn;}

        inline int get_hm_clock(){return hm_clock;}

        inline void change_turn(){turn = (colour)~turn;}

        piece_names get_piece_on_square(int square) const {
            for(auto p: bitboards){
                if(p.second & set_bit(square)){return p.first;}
            }
            
            return None;
        }

        void view_board(){
            std::cout << ((turn == WHITE) ? "white" : "black") << " to move" << std::endl; 
            std::cout << "Half move clock: " << hm_clock << std::endl;
            std::cout << "  =======================" << std::endl;
                                                    
            char letter;
            piece_names name;

            for(int i = 0; i < 64; ++i){
                name = get_piece_on_square(63-i);
                if(name == None){
                    letter = '.';
                } else {
                    letter = name_to_char(name);
                }

                if((i+1) % 8 == 0){
                    std::cout << " " << letter << "| \n";
                }else if(i % 8 == 0){
                    std::cout << 8-(i / 8) << "| " << letter << " ";
                }else{
                    std::cout << " " << letter << " ";
                }
            }
            std::cout << "  ======================="  << std::endl;
            std::cout << "   a  b  c  d  e  f  g  h" << std::endl;
        }

        bool move_is_valid(Move& move){
            std::cout << move.get_move() << std::endl;
            
            std::cout << "Valids" << std::endl;
            for(auto v_move : valid_moves){
                std::cout << v_move.get_move() << std::endl;
                if(v_move == move){return true;}
            }
            
            return false;
        }

        inline void set_piece_bitboard(const piece_names& piece_name, const U64& bitboard) {
            bitboards[piece_name] = bitboard;
        }

        inline colour get_piece_colour(const piece_names& piece_name){
            return (colour)(piece_name > 7);
        }

        inline U64 get_piece_bitboard(const piece_names& piece_name) {
            return bitboards[piece_name];
        }

        U64 get_entire_bitboard() const {
            U64 full_board = 0;

            for(auto p : bitboards){
                full_board |= p.second;
            }
            
            return full_board;
        }

        U64 get_whites(){
            U64 whites = 0;

            for(auto p : bitboards){
                if(p.first < 7){whites |= p.second;}
            }

            return whites;
        }

        U64 get_blacks(){
            U64 blacks = 0;

            for(auto p : bitboards){
                if(p.first > 7){blacks |= p.second;}
            }
            
            return blacks;
        }

        void add_valid_move(const Move& valid_move){
            valid_moves.push_back(valid_move);
        }

        void clear_valid_moves(){
            valid_moves.clear();
        }

        std::vector<Move> get_valid_moves(){
            return valid_moves;
        }

    private:        
        // informaton from fen string
        colour turn;
        uint8_t castling_rights;          
        int hm_clock;
        int fullmoves;
        int ep_square;

        // piece bitboards
        std::unordered_map<piece_names, U64> bitboards{};

        std::vector<uint8_t> castling_rights_history;
    
        std::vector<Move> move_history;
        std::vector<int> hm_clock_reset_history;
        std::vector<piece_names> captured_pieces;

        std::vector<Move> valid_moves;
};

#endif
