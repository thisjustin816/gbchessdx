#include <stdint.h>
#include "chess.h"
#include "eval.h"

int16_t gameScore = 0;
uint16_t posHash;               // Global Position Hash

// --- Material Values ---
// P, N, B, R, Q, K
const int16_t material_value[] = { 0, 100, 320, 330, 500, 900, 20000 };

// --- Piece-Square Tables ---
// Defined for WHITE. Black uses these mirrored vertically.
// Values are small bonuses/penalties to guide the AI.

const int8_t pawn_pst[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    5, 10, 10, -20, -20, 10, 10, 5,
    5, -5, -10, 0, 0, -10, -5, 5,
    0, 0, 0, 30, 30, 0, 0, 0,
    5, 5, 10, 25, 25, 10, 5, 5,
    10, 10, 20, 30, 30, 20, 10, 10,
    50, 50, 50, 50, 50, 50, 50, 50,
    0, 0, 0, 0, 0, 0, 0, 0
};

const int8_t knight_pst[] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20, 0, 0, 0, 0, -20, -40,
    -30, 5, 10, 15, 15, 10, 5, -30,
    -30, 0, 15, 20, 20, 15, 0, -30,
    -30, 5, 15, 20, 20, 15, 5, -30,
    -30, 0, 10, 15, 15, 10, 0, -30,
    -40, -20, 0, 5, 5, 0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50
};

const int8_t bishop_pst[] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10, 5, 0, 0, 0, 0, 5, -10,
    -10, 10, 10, 10, 10, 10, 10, -10,
    -10, 0, 10, 10, 10, 10, 0, -10,
    -10, 5, 5, 10, 10, 5, 5, -10,
    -10, 0, 5, 10, 10, 5, 0, -10,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
};

const int8_t rook_pst[] = {
    0, 0, 0, 5, 5, 0, 0, 0,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    5, 10, 10, 10, 10, 10, 10, 5,
    0, 0, 0, 0, 0, 0, 0, 0
};

// Encourages King to stay in corners/back rank
const int8_t king_pst[] = {
    20, 30, 10, 0, 0, 10, 30, 20,
    20, 20, 0, 0, 0, 0, 20, 20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30
};

// Generic table pointer
const int8_t *get_pst(uint8_t type) {
    switch (type) {
    case PAWN:
        return pawn_pst;
    case KNIGHT:
        return knight_pst;
    case BISHOP:
        return bishop_pst;
    case ROOK:
        return rook_pst;
    case KING:
        return king_pst;
    default:
        return 0;               // Queen has no PST in this simple engine
    }
}

// Simple hash function to mix piece and square into a 16-bit key
uint16_t get_hash_key(uint8_t piece, uint8_t sq) {
    // Arbitrary large primes to scramble the bits
    return (uint16_t) (piece * (sq + 1) * 44203 + 0xACE1);
}

// Get value of a piece at a specific location
int16_t get_piece_value(uint8_t piece, uint8_t sq) {
    uint8_t type = GET_TYPE(piece);
    if (type == EMPTY)
        return 0;

    int16_t val = material_value[type];

    const int8_t *pst = get_pst(type);
    if (pst) {
        uint8_t idx = SQ64(sq);
        // If Black, flip the board vertically (row 0 becomes row 7)
        // In linear 0-63, XORing with 56 (0x38) flips the row
        if (GET_COLOR(piece) == BLACK) {
            idx ^= 56;
        }
        val += pst[idx];
    }

    return val;
}

void init_eval(void) {
    gameScore = 0;
    posHash = 0;
    // Iterate over 0x88 board
    for (uint8_t i = 0; i < BOARD_SIZE; i++) {
        if (!IS_OFFBOARD(i) && board[i] != EMPTY) {
            eval_add_piece(board[i], i);
        }
    }
    // Add side to move to hash (assuming init_board sets sideToMove before calling this)
    if (sideToMove == BLACK)
        posHash ^= 0xFFFF;
}

// Add a piece's value to the global score
void eval_add_piece(uint8_t piece, uint8_t sq) {
    int16_t val = get_piece_value(piece, sq);
    if (GET_COLOR(piece) == WHITE) {
        gameScore += val;
    } else {
        gameScore -= val;
    }
    // Update Hash
    posHash ^= get_hash_key(piece, sq);

}

// Remove a piece's value
void eval_remove_piece(uint8_t piece, uint8_t sq) {
    int16_t val = get_piece_value(piece, sq);
    if (GET_COLOR(piece) == WHITE) {
        gameScore -= val;
    } else {
        gameScore += val;
    }
    // Update Hash
    posHash ^= get_hash_key(piece, sq);
}

// Optimization: Moving is just Remove(old) + Add(new)
// But we can calculate delta directly to save function overhead if needed.
// For now, this is clean.
void eval_move_piece(uint8_t piece, uint8_t from, uint8_t to) {
    eval_remove_piece(piece, from);
    eval_add_piece(piece, to);
}
