#include "../headers/utils.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
struct timeval tv;
#endif

char char_pieces[12] = {'P','K','N','B','R','Q','p','k','n','b','r','q'};

// psqt scores that encode base piece values as well

const int PAWN_PSQT[64] = {
    100,   100,   100,  100,   100,    100,  100,   100,
    150,   150,   150,  150,   150,    150,  150,   150,
    110,   110,   120,  130,   130,    120,  110,   110,
    105,   105,   100,  125,   125,   100,  105,   105,
    100,   100,   100,  120,   120,    100,  100,   100,
    105,   95,    90,   100,   100,    90,   95,    105,
    105,   110,   110,   80,    80,    110,  110,   105,
    100,   100,   100,  100,   100,    100,  100,   100
};

const int ROOK_PSQT[64] = {
    500,   500,   500,   500,   500,   500,   500,   500,
    510,   520,   520,   520,   520,   520,   520,   510,
    450,   500,   500,   500,   500,   500,   500,   450,
    450,   500,   500,   500,   500,   500,   500,   450,
    450,   500,   500,   500,   500,   500,   500,   450,
    450,   500,   500,   500,   500,   500,   500,   450,
    450,   500,   500,   500,   500,   500,   500,   450,
    500,   500,   500,   510,   510,   505,   500,   500
};

const int KNIGHT_PSQT[64] = {
    290,   300,   300,   300,   300,   300,   300,   290,
    300,   305,   305,   305,   305,   305,   305,   300,
    300,   305,   315,   320,   320,   315,   305,   300,
    300,   305,   320,   325,   325,   320,   305,   300,
    300,   305,   320,   325,   325,   320,   305,   300,
    300,   305,   315,   325,   325,   315,   305,   300,
    300,   305,   305,   305,   305,   305,   305,   300,
    290,   310,   300,   300,   300,   300,   310,   290
};

const int BISHOP_PSQT[64] = {
    310,   320,   320,   320,   320,   320,   320,   310,
    320,   330,   330,   330,   330,   330,   330,   320,
    320,   330,   335,   340,   340,   335,   330,   320,
    320,   335,   335,   340,   340,   335,   335,   320,
    320,   330,   340,   340,   340,   340,   330,   320,
    320,   340,   340,   340,   340,   340,   340,   320,
    320,   335,   330,   330,   330,   330,   335,   320,
    310,   320,   320,   320,   320,   320,   320,   310,
};

const int QUEEN_PSQT[64] = {
    870,  890,   890,   895,   895,   890,  890,   870,
    890,  900,   900,   900,   900,   900,  900,   890,
    890,  900,   905,   905,   905,   905,  900,   890,
    895,  900,   905,   905,   905,   905,  900,   895,
    900,  900,   905,   905,   905,   905,  900,   895,
    890,  905,   905,   905,   905,   905,  900,   890,
    890,  900,   905,   900,   900,   900,  900,   890,
    870,  890,   890,   895,   895,   890,  890,   870
};

const int KING_PSQT[64] = {
    4970,  4960,   4960,   4950,   4950,   4960,   4960,   4970,
    4970,  4960,   4960,   4950,   4950,   4960,   4960,   4970,
    4970,  4960,   4960,   4950,   4950,   4960,   4960,   4970,
    4970,  4960,   4960,   4950,   4950,   4960,   4960,   4970,
    4980,  4970,   4970,   4960,   4960,   4970,   4970,   4980,
    4990,  4980,   4980,   4980,   4980,   4980,   4980,   4990,
    5020,  5020,   5000,   5000,   5000,   5000,   5020,   5020,
    5020,  5030,   5010,   5000,   5000,   5010,   5030,   5020
};

const int* PIECE_VALUES[12] = {PAWN_PSQT, KING_PSQT, KNIGHT_PSQT, BISHOP_PSQT, ROOK_PSQT, QUEEN_PSQT, PAWN_PSQT, KING_PSQT, KNIGHT_PSQT, BISHOP_PSQT, ROOK_PSQT, QUEEN_PSQT};

int FLIP[2][64];

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

/// @brief Indexing matches colours (0 for white, 1 for black)
castling_and_enpassant_info cep_info[2] = {
    {
    .kcr = K_castle,
    .qcr = Q_castle,
    .rook_to_move = R,
    .ally_king = K,
    .ep_sq_offset = -8,
    .ep_pawn = p,
    .rook_kingside_sq = h1,
    .rook_queenside_sq = a1,
    },

    {
    .kcr = k_castle,
    .qcr = q_castle,
    .rook_to_move = r,
    .ally_king = k,
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

    if(move_type & 0x8){
        printf("%s%s%c ", from, to, char_pieces[(move_type & 0x3) + 8]);
    } else {
        printf("%s%s ", from, to);
    }   
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

int maxi(int a, int b){
    return (a > b) ? a : b;
}

int mini(int a, int b){
    return (a < b) ? a : b;
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

void init_flip(){
    int sq;

    for(sq = 0; sq < 64; ++sq){
        FLIP[BLACK][sq] = sq;
    }

    for(sq = 0; sq < 64; ++sq){
        FLIP[WHITE][sq] = 63 - sq;
    }

}
