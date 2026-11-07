#ifndef GB_CHESS_RULES_CORE_H
#define GB_CHESS_RULES_CORE_H
#include <stdint.h>
/* Research candidate: portable 0x88 rules, independent of the pinned AI.
 * Game Boy callers own the legal-move buffer and scratch position;
 * no dynamic allocation and no use of the hardware stack for move arrays.
 * FEN I/O and perft live in host-only test utilities.
 */
#define RC_MAX_MOVES 256
#define RC_EMPTY 0
#define RC_PAWN 1
#define RC_KNIGHT 2
#define RC_BISHOP 3
#define RC_ROOK 4
#define RC_QUEEN 5
#define RC_KING 6
#define RC_WHITE 0
#define RC_BLACK 8
#define RC_WK 1
#define RC_WQ 2
#define RC_BK 4
#define RC_BQ 8
#define RC_EP 1
#define RC_CASTLE 2
#define RC_NONE 255

typedef struct {
    uint8_t board[128];
    uint8_t side, castle, ep;
    uint16_t halfmove, fullmove;
} RcPosition;
typedef struct {
    uint8_t from, to, promotion, flags;
} RcMove;

void rc_init(RcPosition *p);
uint8_t rc_attacked(const RcPosition *p,uint8_t square,uint8_t by_color);
uint8_t rc_in_check(const RcPosition *p,uint8_t color);
void rc_apply(const RcPosition *p,RcMove m,RcPosition *next);
/* Compact state restoration for Coaching takebacks and game-history replay. */
typedef struct {
    uint8_t captured,castle,ep;
    uint16_t halfmove,fullmove;
} RcUndo;
void rc_make_inplace(RcPosition *p,RcMove m,RcUndo *undo);
void rc_unmake_inplace(RcPosition *p,RcMove m,const RcUndo *undo);
/* Returns a nonnegative count; -1 for insufficient buffer capacity. */
int16_t rc_legal(const RcPosition *p,RcMove *out,uint16_t capacity,RcPosition *scratch);
uint8_t rc_same_repetition_state(const RcPosition *a,const RcPosition *b,RcMove *scratch_moves,RcPosition *scratch_position);
/* Incremental, no heap: each call processes one board square's pawn,
 * knight or king moves (after passing over the empty and enemy squares
 * before it in the same rank), one ray of a bishop, rook or queen, one castling
 * attack test, one rank of the search for the mover's king, or one
 * pseudo-legal candidate, so no call makes more than one rc_attacked test,
 * walks more than one ray, or scans more than one rank. Moves are emitted in the
 * same order as rc_legal. The maximum cached moves from any single piece is 27.
 * Returns 0=more work, 1=legal move emitted, 2=done, 3=invalid capacity.
 * Board must remain unchanged until the iterator completes.
 */
#define RC_PIECE_MOVES 32
typedef struct {
    RcMove candidates[RC_PIECE_MOVES];
    uint8_t square, offset, count, stage, king, kscan, ray, rays, filter;
} RcIterator;
#define RC_NO_KING 0x88u
#ifdef RC_STEP_PROBE
/* Host test hook: counts rc_attacked calls. */
extern unsigned long rc_probe_attack_tests;
#endif
void rc_iter_begin(RcIterator *iterator);
/* The same iterator emitting only one class of legal move, in the same
 * relative order: RC_ITER_TACTICAL (captures, en passant, promotions) or
 * RC_ITER_QUIET (everything else). One pass of each emits every legal move
 * once. */
#define RC_ITER_ALL 0u
#define RC_ITER_TACTICAL 1u
#define RC_ITER_QUIET 2u
void rc_iter_begin_filter(RcIterator *iterator, uint8_t filter);
uint8_t rc_iter_step(const RcPosition *p,RcIterator *iterator,RcPosition *scratch,RcMove *out);

#endif
