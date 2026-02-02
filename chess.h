// --- Constants & Definitions ---

#define BOARD_SIZE 128
#define MAX_MOVES 80

// Pieces
#define EMPTY 0
#define PAWN 1
#define KNIGHT 2
#define BISHOP 3
#define ROOK 4
#define QUEEN 5
#define KING 6

#define COLOR_MASK 0x08
#define WHITE 0x00
#define BLACK 0x08

// Castling Bits
#define WK_CASTLE 1             // White King Side
#define WQ_CASTLE 2             // White Queen Side
#define BK_CASTLE 4             // Black King Side
#define BQ_CASTLE 8             // Black Queen Side

// Macros
#define IS_OFFBOARD(sq) ((sq) & 0x88)
#define GET_COLOR(piece) (piece & COLOR_MASK)
#define GET_TYPE(piece) (piece & 0x07)
#define OPPONENT(color) (color ^ BLACK)

// --- Data Structures ---
typedef struct {
    uint8_t from;
    uint8_t to;
    uint8_t captured;
    uint8_t special;            // 0=Normal, 1=EP Capture, 2=Castle
    uint8_t old_castle;         // To restore state
    uint8_t old_ep;             // To restore state
} Move;

// Global State
extern uint8_t board[BOARD_SIZE];       // 0x88 Board
extern uint8_t sideToMove;      // WHITE or BLACK
extern uint8_t castleRights;    // Bitmask: 0000 KQkq
extern uint8_t epSquare;        // 0 if no Enpassant, otherwise the square behind the pawn

// Functions
void init_board(void);
uint8_t generate_moves(Move * moveList);
// Returns 0 if legal, 1 if illegal (leaves King in check)
uint8_t make_move(Move * m);
void unmake_move(Move * m);
void print_board(void);
uint8_t checkmate_or_stalemate(void);
