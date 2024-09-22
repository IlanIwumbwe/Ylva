#include "../headers/defs.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
struct timeval tv;
#endif

namespace fs = std::filesystem;

bool debug = false;

dirInfo dir_info[8] = {
    {north, 8, RANK(8), south},
    {east, -1, H_FILE, west},
    {west, 1, A_FILE, east},
    {south, -8, RANK(1), north},
    {noEa, 7, H_FILE | RANK(8), soWe},
    {noWe, 9, A_FILE  | RANK(8), soEa},
    {soEa, -9, H_FILE | RANK(1), noWe},
    {soWe, -7, A_FILE | RANK(1), noEa}    
};

std::unordered_map<char, piece_names> char_to_name = {
    {'P', P},
    {'K', K},
    {'Q', Q},
    {'R', R},
    {'N', N},
    {'B', B},
    {'p', p},
    {'k', k},
    {'q', q},
    {'r', r},
    {'n', n},
    {'b', b}
};

std::unordered_map<std::string, uint> promo_flags = {
    {"q", 11},
    {"r", 10},
    {"n", 8},
    {"b", 9},
    {"qc", 15},
    {"rc", 14},
    {"nc", 12},
    {"bc", 13}
};

std::unordered_map<uint, std::string> print_promo_flag = {
    {8, "n"},
    {12, "n"},
    {9, "b"},
    {13, "b"},
    {10, "r"},
    {14, "r"},
    {11, "q"},
    {15, "q"}
};

int get_piece_value[7] = {0, 100, 0, 900, 500, 300, 300};

uint p_flags[4] = {8,9,10,11};
uint pc_flags[4] = {12,13,14,15};
U64 knight_attack_set[64] = {};
U64 king_attack_set[64] = {};
U64 RAYS[8][64] = {};

void populate_rays(){
    for(auto info : dir_info){
        for(int i = 0; i < 64; ++i){
            U64 out = 0;
            int mult = 0;

            out |= set_bit(i);

            while((out & info.bound) == 0){
                mult ++;
                out |= set_bit(i + mult * info.offset);
            }

            out &= ~set_bit(i);

            RAYS[info.dir][i] = out;
        }
    }
}

void populate_attack_sets(){
    for(int i = 0; i < 64; ++i){
        knight_attacks(set_bit(i), knight_attack_set[i]);
        king_attacks(set_bit(i), king_attack_set[i]);
    }
}

std::string name_to_char(const piece_names& name){ 
    switch(name){
        case P : return "P";
        case p : return "p";
        case Q : return "Q";
        case q : return "q";
        case N : return "N";
        case n : return "n";
        case R : return "R";
        case r : return "r";
        case B : return "B";
        case b : return "b";
        case K : return "K";
        case k : return "k";
        case None : return ".";
        default :
            std::cout << "What? The piece name (piece_name) " << name << " does not exist" << std::endl;
            exit(0);
    }
}

/// Produce bitboard of all attacked squares by a knight at given square
void knight_attacks(U64 bitboard, U64& output){
    output = 0;

    output |= (bitboard & ~(G_FILE | H_FILE | RANK(8))) << 6;
    output |= (bitboard & ~(RANK(7) | RANK(8) | H_FILE)) << 15;
    output |= (bitboard & ~(A_FILE | B_FILE | RANK(8))) << 10;
    output |= (bitboard & ~(A_FILE | RANK(7) | RANK(8))) << 17;
    output |= (bitboard & ~(G_FILE | H_FILE | RANK(1))) >> 10;
    output |= (bitboard & ~(H_FILE | RANK(1) | RANK(2))) >> 17;
    output |= (bitboard & ~(A_FILE | B_FILE | RANK(1))) >> 6;
    output |= (bitboard & ~(A_FILE | RANK(1) | RANK(2))) >> 15;
}

/// Produce bitboard of all attacked squares by a king at given square
void king_attacks(U64 bitboard, U64& output){
    output = 0;
    
    output |= (bitboard & ~RANK(8)) << 8;
    output |= (bitboard & ~H_FILE) >> 1;
    output |= (bitboard & ~A_FILE) << 1;
    output |= (bitboard & ~RANK(1)) >> 8;
    output |= (bitboard & ~(RANK(8) | H_FILE)) << 7;
    output |= (bitboard & ~(RANK(8) | A_FILE)) << 9;
    output |= (bitboard & ~(RANK(1) | H_FILE)) >> 9;
    output |= (bitboard & ~(RANK(1) | A_FILE)) >> 7;
}

std::vector<std::string> splitString(const std::string& input, const char& delimiter) {
    std::istringstream iss(input);
    std::vector<std::string> parts;
    std::string part;
    
    while (std::getline(iss, part, delimiter)) {
        parts.push_back(part);
    }

    return parts;
}

std::string removeWhiteSpace(std::string str){
    std::regex pattern(R"(^\s+|\s+$|\t|\n)");

    return std::regex_replace(str, pattern, "");
}

bool isStringDigit(std::string& input){
    for(auto c : input){
        if(!isdigit(c)){
            return false;
        }
    }

    return true;

}

inline auto numtobin(const U64& number){
    std::bitset<64> binaryRepresentation(number);

    return binaryRepresentation;
}

void printbitboard(const U64& number){
    auto bin = numtobin(number);
    std::cout << "  ------------------------" << std::endl;
    for(int i = 0; i < 64; ++i){
        if((i+1) % 8 == 0){
            std::cout << " " << bin.to_string()[i] << " \n";
        }else if(i % 8 == 0){
            std::cout << "   " << bin.to_string()[i] << " ";
        }else{
            std::cout << " " << bin.to_string()[i] << " ";
        }
    }
    std::cout << "  ------------------------" << std::endl;
}

uint alg_to_int(const std::string& square){
    return 8*((square[1]-'0')-1) + (104-square[0]);
}


std::string int_to_alg(const uint& square){
    char fileind = 104 - (square % 8);
    int rankind = (square / 8) + 1;

    std::string out(1,fileind);
    out += std::to_string(rankind);
    return out;
}

bool is_popcnt_supported() {
    static bool checked = false;
    static bool supported = false;
    if (!checked) {
        unsigned int eax, ebx, ecx, edx;
        // Call CPUID with eax = 1 to get feature information in ecx
        if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
            supported = ecx & (1 << 23); // Check bit 23 of ecx
        }
        checked = true;
    }
    return supported;
}


uint count_set_bits(U64 bitboard){
    if(is_popcnt_supported()){
        return _mm_popcnt_u64(bitboard);
    } else  {
        uint count = 0;
        for(; bitboard; ++count){
            bitboard &= bitboard-1;
        }

        return count;
    }
}

/// @brief Given a piece as integer, map it to an index to a 7 element array
/// @param piece 
/// @return 
int convert_piece_to_index(int piece) {
    return piece % 8;
}

int convert_piece_to_zobrist_index(int piece){
    return (piece > 7) ? piece - 2 : piece;
}

/// @brief Given a square, return an index that can access the value from piece square tables. 
/// @param square, colour_index 
/// @return piece square tables index (uint)
uint convert_square_to_index(int square, int colour_index){
    //colour_index = 1-colour_index;
    square = std::abs(63*colour_index - square);

    int file = square % 8;
    int rank = square / 8;

    return 4*rank + std::min(7-file, file);
}

/// @brief Dynamically allocate number of entries for the PV table based on the size in bytes required
/// @param table 
/// @param PV_size 
void init_pv_table(PV_Table* table, int PV_size){
    table->num_of_entries = PV_size / sizeof(PV_entry);
    table->num_of_entries -= 2; 
    free(table->pv_entries);  
    table->pv_entries = (PV_entry*) malloc(table->num_of_entries * sizeof(PV_entry));
    clear_pv_table(table);
    
    if(debug){
        std::cout << "info string initialised PV table with " << table->num_of_entries << " entries " << std::endl;
    }
}

/// @brief Set all position hash keys and moves to 0 (0ULL for hash as it is 64 bit)
/// @param table 
void clear_pv_table(PV_Table* table){
    PV_entry* entry;

    for(entry = table->pv_entries; entry < table->pv_entries + table->num_of_entries; ++entry){
        entry->hash_key = 0ULL;
        entry->move = 0;
    }

    if(debug){std::cout << "info string cleared PV table " << std::endl;}
}

std::vector<std::string> get_tokens(std::string& input, std::regex pattern){
    std::vector<std::string> tokens;

    std::sregex_iterator iter(input.begin(), input.end(), pattern);
    std::sregex_iterator end;

    while (iter != end) {
        tokens.push_back(iter->str());
        ++iter;
    }

    return tokens;
}

bool is_valid_num(const std::string& numString){
    auto pattern = std::regex(R"([0-9]+)");

    return std::regex_match(numString, pattern);
}

#ifdef _WIN32
U64 time_in_ms(){
    return GetTickCount();
}

int peek_stdin(){
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