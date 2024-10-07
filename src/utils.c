#include "../headers/utils.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
struct timeval tv;
#endif

char char_pieces[12] = {'P','K','N','B','R','Q','p','k','n','b','r','q'};

char* char_squares[64] = {
    "h1", "g1", "f1", "e1", "d1", "c1", "b1", "a1", 
    "h2", "g2", "f2", "e2", "d2", "c2", "b2", "a2", 
    "h3", "g3", "f3", "e3", "d3", "c3", "b3", "a3", 
    "h4", "g4", "f4", "e4", "d4", "c4", "b4", "a4", 
    "h5", "g5", "f5", "e5", "d5", "c5", "b5", "a5", 
    "h6", "g6", "f6", "e6", "d6", "c6", "b6", "a6", 
    "h7", "g7", "f7", "e7", "d7", "c7", "b7", "a7", 
    "h8", "g8", "f8", "e8", "d8", "c8", "b8", "a8", 
};

int file(char c){
    return 104 - c;
}

int rank(char c){
    return c - '0' - 1;
}

square sq(char* sq){
    return 8 * rank(sq[1]) + file(sq[0]);
}

#define rank(c) (c - '0' - 1)

/// @brief Indexing matches colours (0 for white, 1 for black)
castling_and_enpassant_info cep_info[2] = {
    {
    .kcr = K_castle,
    .qcr = Q_castle,
    .rook_to_move = R,
    .ep_sq_offset = -8,
    .ep_pawn = p,
    .rook_kingside_sq = h1,
    .rook_queenside_sq = a1,
    },

    {
    .kcr = k_castle,
    .qcr = q_castle,
    .rook_to_move = r,
    .ep_sq_offset = 8,
    .ep_pawn = P,
    .rook_kingside_sq = h8,
    .rook_queenside_sq = a8,
    },
};

piece char_to_piece(const char c_piece){

    for(int i = 0; i < 12; ++i){
        if(char_pieces[i] == c_piece){
            return i;
        } 
    }

    return p_none;
}

square char_to_square(const char* c_square){

    for(int i = 0; i < 64; ++i){
        if(strcmp(char_squares[i], c_square) == 0){
            return i;
        } 
    }

    return s_none;
}

void print_bitboard(const U64 number){
    U64 mask = set_bit(63);
    int i = 0;

    printf("number: %lu\n", number);

    while(mask){
        if(mask & number){
            printf("1 ");
        } else {
            printf("0 ");
        }

        mask = mask >> 1;
        i++;

        if((i % 8) == 0){
            printf("\n");
        }
    }

    printf("\n");
}

int count_set_bits(U64 number){
    int i = 0;
    for(; number; ++i){
        number &= (number - 1);
    }

    return i;
}

int tokenise(char* string, char* output[]){
    char* t = strtok(strdup(string), " ");
    int i = 0;

    while(t != NULL){
        output[i++] = t;   
        t = strtok(NULL, " ");
    }

    return i;
}

void print_move(U16 move){
    int move_type = move_type(move);

    char* from = char_squares[move_from_square(move)];
    char* to = char_squares[move_to_square(move)];
    char promo = (move_type & 0x8) ? char_pieces[(move_type & 0x3) + 8] : ' ';

    printf("%s%s%c", from, to, promo);
    
}

// modify move type mask based on the promotion type of the move
void set_promotion_type(char* c, int* m_type){

    if(c){
        switch(*c){
            case 'n': *m_type |= 0x8 ; break;
            case 'b': *m_type |= 0x8 | 0x1; break;
            case 'r': *m_type |= 0x8 | 0x2; break;
            case 'q': *m_type |= 0x8 | 0x3; break;
        }
    }

}

/// @brief converts str move into U16
/// @param move 
/// @return 
U16 move_from_str(char* move){

    square from = sq(move) & 0x3f;
    square to = sq(move+2) & 0x3f;
    int m_type = 0x0;

    // printf("from : %d , to : %d\n", from, to);

    piece p_from = piece_on_square(from);
    piece p_to = piece_on_square(to);

    set_promotion_type(move+4, &m_type);

    int square_dist = from - to;
    
    if(p_to != p_none){
        m_type |= 0x4;
    }

    if(m_type) goto end;

    if((p_from == P) || (p_from == p)){
        if(abs(square_dist) == 9 || abs(square_dist) == 7){
            m_type = 0x5;
        } else if(square_dist == 16 || square_dist == -16){
            m_type = 0x1;
        }
    } else if ((p_from == K) || (p_from == k)){
        if(square_dist == 2){
            m_type = 0x2;
        } else if(square_dist == 3){
            m_type = 0x3;
        }
    }

    end:
    return (m_type << 12) | (from << 6) | to;
}

#ifdef _WIN32

U64 time_in_ms(){
    return GetTickCount();
}

int input_waiting(){
    static int init = 0, pipe;
    static HANDLE inh;
    DWORD dw;

    if (!init) {
        init = 1;
        inh = GetStdHandle(STD_INPUT_HANDLE);
        pipe = !GetConsoleMode(inh, &dw);
    }
    if (pipe) {
        if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
        return dw;
    } else return _kbhit();
}
#else
U64 time_in_ms() {
	gettimeofday (&tv, NULL);
	return(tv.tv_sec * 1000LL + (tv.tv_usec / 1000));
}

int input_waiting(){
    fd_set readfds;

	FD_ZERO (&readfds);
	FD_SET (fileno(stdin), &readfds);
	tv.tv_sec=0; tv.tv_usec=0;
	select(16, &readfds, 0, 0, &tv);

	return (FD_ISSET(fileno(stdin), &readfds));
}
#endif