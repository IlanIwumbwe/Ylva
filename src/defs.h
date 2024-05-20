#ifndef DEFS_H
#define DEFS_H

#include <cstdint>
#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <regex>
#include <cmath>
#include <assert.h>

typedef uint64_t U64;
typedef unsigned int uint;

#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define MOVE_FORMAT std::regex(R"([a-h][1-8][a-h][1-8](q|r|n|b)?)")
#define VERSION_FORMAT std::regex(R"(v[0-2])")
#define UCI_COMMAND_FORMAT std::regex(R"(([A-Za-z1-8]+/?){8} [wb] ([KQkq]+|-) ([a-f1-8]+|-) [0-9]+ [1-9]+|[a-z0-9]+|[a-z]+|[0-9]+)")

#define CAPTURE_FLAG 0x4000
#define PROMO_FLAG 0x8000

#define A_FILE 0x8080808080808080
#define B_FILE 0x4040404040404040
#define G_FILE 0x0202020202020202
#define H_FILE 0x0101010101010101   
#define RANK(rank_num) ((uint64_t)0xff << (rank_num-1)*8)

#define K_castle 0x8
#define Q_castle 0x4
#define k_castle 0x2
#define q_castle 0x1        

#define get_lsb(bitboard) __builtin_ctzll(bitboard)
#define get_msb(bitboard) 64 - (__builtin_clzll(bitboard)+1)
#define set_bit(i) (1ULL << (i))
#define get_bit(bitboard, i) (bitboard & set_bit(i))

#define infinity std::numeric_limits<int>::max()

#define MAX_DEPTH 64

// macro to generate a random 64 bit number since std::rand gives 16 bit number

#define RAND64 ((U64)std::rand() + \
                ((U64)std::rand() << 15) + \
                ((U64)std::rand() << 30) + \
                ((U64)std::rand() << 45) + \
                (((U64)std::rand() & 0xf) << 60)) 

extern bool debug;

/// @brief A single PV entry stores the best move in a position identified by its hash key
struct PV_entry {
    uint16_t move;
    U64 hash_key;
};

/// @brief Pointer to memory allocated for PV entries by malloc
struct PV_Table {
    PV_entry* pv_entries;
    int num_of_entries;
};

void init_pv_table(PV_Table* table, int PV_size);

void clear_pv_table(PV_Table* table);

typedef enum {
    None = 0,
    P = 1 , 
    K = 2 ,
    Q = 3 , 
    R = 4 ,
    N = 5 ,
    B = 6 ,
    p = 9 ,
    k = 10,
    q = 11,
    r = 12,
    n = 13,
    b = 14,
} piece_names;

typedef enum{
    WHITE = 0,
    BLACK = -1,
} colour;

typedef enum{PVE, EVE, PVP, UCI} game_modes;
 
typedef enum{north, east, west, south, noEa, soEa, noWe, soWe} dirs;

typedef enum {diag, nondiag} ray_type;

struct dirInfo {
    dirs dir;
    int offset;
    U64 bound;
    dirs opp_dir;
}; 

extern dirInfo dir_info[8];

extern std::unordered_map<char, piece_names> char_to_name;

extern std::unordered_map<std::string, uint> promo_flags;

extern std::unordered_map<uint, std::string> print_promo_flag;

extern int get_piece_value[7];

extern uint p_flags[4];
extern uint pc_flags[4];

// attack sets
extern U64 knight_attack_set[64];
extern U64 king_attack_set[64];

extern U64 RAYS[8][64];

extern const int* PSQT[6];

void populate_attack_sets();

void populate_rays();

std::string name_to_char(const piece_names& name);

void knight_attacks(U64 bitboard, U64& output);

void king_attacks(U64 bitboard, U64& output);

std::vector<std::string> splitString(const std::string& input, const char& delimiter);

std::string get_first(const std::string& input, const char& delimiter);

bool get_next_uci_param(const std::string& input, std::string& substr, std::string other_word, size_t from);

std::string removeWhiteSpace(std::string str);

bool isStringDigit(std::string& input);

auto numtobin(const U64& number);

void printbitboard(const U64& number);

uint alg_to_int(const std::string& square);

int rank(const uint& square);

int file(const uint& square);

std::string int_to_alg(const uint& square);

uint count_set_bits(U64 bitboard);

int convert_piece_to_index(int piece);

int convert_piece_to_zobrist_index(int piece);

uint convert_square_to_index(uint square, int colour_index);

std::vector<std::string> get_tokens(std::string& input, std::regex pattern);

bool is_valid_num(const std::string& numString);

#endif
