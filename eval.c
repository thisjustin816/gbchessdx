#include <stdint.h>
#include <stdlib.h>
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
    20, 20, 30, 40, 40, 30, 20, 20,     // Rank 6: Increased center to 40
    80, 80, 80, 80, 80, 80, 80, 80,     // Rank 7: Increased from 50 to 80 (Huge incentive)
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

// Helper: Manhattan distance (moves to reach square)
uint8_t distance(uint8_t sq1, uint8_t sq2) {
    int8_t r1 = (int8_t) (sq1 >> 4), c1 = sq1 & 7;
    int8_t r2 = (int8_t) (sq2 >> 4), c2 = sq2 & 7;
    return (uint8_t) (abs(r1 - r2) + abs(c1 - c2));
}

// Helper: Distance from center (for pushing king to edge)
// Center is 3.5, We scale up to avoid floats.
uint8_t dist_from_center(uint8_t sq) {
    int8_t r = (int8_t) (sq >> 4), c = sq & 7;
    // Rank dist: 0->3, 1->2, 2->1, 3->0, 4->0, 5->1 ...
    int8_t r_dist = (int8_t) abs(r * 2 - 7);
    int8_t c_dist = (int8_t) abs(c * 2 - 7);
    return (uint8_t) (r_dist + c_dist);
}

int16_t get_final_eval(void) {
    int16_t score = gameScore;

    // Track kings and material to detect endgame phase
    uint8_t wKing = 0, bKing = 0;
    int16_t non_pawn_material = 0;

    // We scan the board to find Kings (already doing this), 
    // but now we also sum material to detect the "Endgame Phase".
    for (uint8_t i = 0; i < BOARD_SIZE; i++) {
        if (IS_OFFBOARD(i))
            continue;

        uint8_t piece = board[i];
        if (piece != EMPTY) {
            uint8_t type = GET_TYPE(piece);
            if (type == KING) {
                if (GET_COLOR(piece) == WHITE)
                    wKing = i;
                else
                    bKing = i;
            } else if (type != PAWN) {
                // Count Knights, Bishops, Rooks, Queens
                non_pawn_material += material_value[type];
            }
        }
    }

    // ENDGAME TRIGGER:
    // If we have a massive lead (>500), standard mop-up.
    // OR: If pieces have been traded off (Material < 1500), enter endgame mode.
    //    1500 is roughly a Rook + Minor piece per side. 
    //    This ensures we play K+P vs K correctly.
    if (score > 500 || score < -500 || non_pawn_material < 1500) {

        int16_t mop_up_score = 0;

        // Determine who is attacking
        uint8_t losingKing = (score > 0) ? bKing : wKing;
        uint8_t winningKing = (score > 0) ? wKing : bKing;

        // Push losing king to edge
        mop_up_score += (int16_t) (dist_from_center(losingKing) * 10);

        // Bring winning king closer to enemy king
        mop_up_score += (int16_t) ((14 - distance(winningKing, losingKing)) * 4);

        // KING ACTIVATION (Crucial for Endgames):
        // The standard King PST encourages hiding in the corner.
        // In the endgame, we want the King in the center!
        // We add a bonus for the attacking King being near the center.
        mop_up_score += (int16_t) ((7 - dist_from_center(winningKing)) * 10);

        // Apply bonus to the side with the advantage
        if (score > 0)
            score += mop_up_score;
        else
            score -= mop_up_score;
    }

    return score;
}
