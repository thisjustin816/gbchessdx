/* * 0x88 Chess Engine
 * Compatible with GBDK-2000 and Standard C
 */

// --- Platform Detection & Types ---
#ifdef __TARGET_gb
#include <gb/gb.h>
#include <stdio.h>
    // This only prints the bottom 16 bits
#define PRIu32 "u"
#else
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
typedef int8_t BYTE;
typedef uint8_t UBYTE;
typedef int16_t WORD;
typedef uint16_t UWORD;
#endif

// --- Constants & Definitions ---

#define BOARD_SIZE 128
#define MAX_MOVES 256           // Increased to be safe

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
    UBYTE from;
    UBYTE to;
    UBYTE captured;
    UBYTE special;              // 0=Normal, 1=EP Capture, 2=Castle
    UBYTE old_castle;           // To restore state
    UBYTE old_ep;               // To restore state
} Move;

// Global State
UBYTE board[BOARD_SIZE];
UBYTE sideToMove = WHITE;
UBYTE castleRights = 0;         // Bitmask: 0000 KQkq
UBYTE epSquare = 0;             // 0 if no EP, otherwise the square behind the pawn

// --- Lookup Data ---

const BYTE knight_offsets[] = { -33, -31, -18, -14, 14, 18, 31, 33 };
const BYTE king_offsets[] = { -17, -16, -15, -1, 1, 15, 16, 17 };
const BYTE bishop_offsets[] = { -17, -15, 15, 17 };
const BYTE rook_offsets[] = { -16, -1, 1, 16 };

// --- Helper Functions ---

// Check if a specific square is attacked by 'side'
UBYTE is_square_attacked(UBYTE sq, UBYTE side) {
    // side is the color of the ATTACKER (e.g. if White King is at sq, side is BLACK)

    // Check Pawn Attacks
    // If side is WHITE, they attack from "below" (sq-17, sq-15)
    // If side is BLACK, they attack from "above" (sq+17, sq+15)
    if (side == WHITE) {
        if (!IS_OFFBOARD(sq - 17) && (board[sq - 17] == (WHITE | PAWN)))
            return 1;
        if (!IS_OFFBOARD(sq - 15) && (board[sq - 15] == (WHITE | PAWN)))
            return 1;
    } else {
        if (!IS_OFFBOARD(sq + 17) && (board[sq + 17] == (BLACK | PAWN)))
            return 1;
        if (!IS_OFFBOARD(sq + 15) && (board[sq + 15] == (BLACK | PAWN)))
            return 1;
    }

    // Check Knights
    UBYTE i;
    for (i = 0; i < 8; i++) {
        UBYTE target = sq - knight_offsets[i];  // Reverse offset check
        if (!IS_OFFBOARD(target)) {
            if (board[target] == (side | KNIGHT))
                return 1;
        }
    }

    // Check Kings
    for (i = 0; i < 8; i++) {
        UBYTE target = sq - king_offsets[i];
        if (!IS_OFFBOARD(target)) {
            if (board[target] == (side | KING))
                return 1;
        }
    }

    // Check Sliders (Rook/Queen)
    for (i = 0; i < 4; i++) {
        BYTE offset = rook_offsets[i];
        UBYTE target = sq + offset;
        while (!IS_OFFBOARD(target)) {
            UBYTE piece = board[target];
            if (piece != EMPTY) {
                if ((piece == (side | ROOK)) || (piece == (side | QUEEN)))
                    return 1;
                break;          // Blocked by any piece
            }
            target += offset;
        }
    }

    // Check Sliders (Bishop/Queen)
    for (i = 0; i < 4; i++) {
        BYTE offset = bishop_offsets[i];
        UBYTE target = sq + offset;
        while (!IS_OFFBOARD(target)) {
            UBYTE piece = board[target];
            if (piece != EMPTY) {
                if ((piece == (side | BISHOP)) || (piece == (side | QUEEN)))
                    return 1;
                break;          // Blocked
            }
            target += offset;
        }
    }

    return 0;
}

void init_board(void) {
    UBYTE i;
    for (i = 0; i < BOARD_SIZE; i++)
        board[i] = EMPTY;

    // Pawns
    for (i = 0; i < 8; i++) {
        board[0x10 + i] = WHITE | PAWN;
        board[0x60 + i] = BLACK | PAWN;
    }

    // Pieces
    board[0x00] = board[0x07] = WHITE | ROOK;
    board[0x01] = board[0x06] = WHITE | KNIGHT;
    board[0x02] = board[0x05] = WHITE | BISHOP;
    board[0x03] = WHITE | QUEEN;
    board[0x04] = WHITE | KING;

    board[0x70] = board[0x77] = BLACK | ROOK;
    board[0x71] = board[0x76] = BLACK | KNIGHT;
    board[0x72] = board[0x75] = BLACK | BISHOP;
    board[0x73] = BLACK | QUEEN;
    board[0x74] = BLACK | KING;

    sideToMove = WHITE;
    castleRights = WK_CASTLE | WQ_CASTLE | BK_CASTLE | BQ_CASTLE;
    epSquare = 0;
}

// --- Move Generation & Execution ---

UBYTE generate_moves(Move *moveList) {
    UBYTE count = 0;
    UBYTE sq, piece, type, color;
    BYTE i, offset;
    UBYTE target_sq;

    for (sq = 0; sq < BOARD_SIZE; sq++) {
        if (IS_OFFBOARD(sq)) {
            sq += 7;
            continue;
        }

        piece = board[sq];
        if (piece == EMPTY || GET_COLOR(piece) != sideToMove)
            continue;

        type = GET_TYPE(piece);
        color = GET_COLOR(piece);

        if (type == PAWN) {
            BYTE dir = (color == WHITE) ? 16 : -16;
            UBYTE start_rank = (color == WHITE) ? 1 : 6;

            // Forward 1
            if (board[sq + dir] == EMPTY) {
                moveList[count].from = sq;
                moveList[count].to = sq + dir;
                moveList[count].captured = EMPTY;
                moveList[count].special = 0;
                count++;

                // Forward 2
                if ((sq >> 4) == start_rank && board[sq + dir + dir] == EMPTY) {
                    moveList[count].from = sq;
                    moveList[count].to = sq + dir + dir;
                    moveList[count].captured = EMPTY;
                    moveList[count].special = 0;        // EP flag set in make_move logic via distance
                    count++;
                }
            }

            // Captures (Normal + En Passant)
            BYTE cap_offsets[] = { 15, 17 };
            for (i = 0; i < 2; i++) {
                offset = (color == WHITE) ? cap_offsets[i] : -cap_offsets[i];
                target_sq = sq + offset;
                if (IS_OFFBOARD(target_sq))
                    continue;

                UBYTE target = board[target_sq];
                if (target != EMPTY && GET_COLOR(target) != color) {
                    moveList[count].from = sq;
                    moveList[count].to = target_sq;
                    moveList[count].captured = target;
                    moveList[count].special = 0;
                    count++;
                }
                // En Passant
                else if (target_sq == epSquare && epSquare != 0) {
                    moveList[count].from = sq;
                    moveList[count].to = target_sq;
                    moveList[count].captured = EMPTY;   // Capture logic handled specially
                    moveList[count].special = 1;        // Mark as EP
                    count++;
                }
            }
        } else if (type == KING) {
            // Normal Moves
            for (i = 0; i < 8; i++) {
                target_sq = sq + king_offsets[i];
                if (IS_OFFBOARD(target_sq))
                    continue;
                if (board[target_sq] == EMPTY || GET_COLOR(board[target_sq]) != color) {
                    moveList[count].from = sq;
                    moveList[count].to = target_sq;
                    moveList[count].captured = board[target_sq];
                    moveList[count].special = 0;
                    count++;
                }
            }


            // Castling
            if (color == WHITE) {
                // Check if King is currently in check (Cannot castle out of check)
                // Note: We check if WHITE King (0x04) is attacked by BLACK
                if (!is_square_attacked(0x04, BLACK)) {
                    // Kingside
                    if ((castleRights & WK_CASTLE) && board[0x05] == EMPTY && board[0x06] == EMPTY) {
                        if (!is_square_attacked(0x05, BLACK) && !is_square_attacked(0x06, BLACK)) {
                            moveList[count].from = 0x04;
                            moveList[count].to = 0x06;
                            moveList[count].captured = EMPTY;
                            moveList[count].special = 2;
                            count++;
                        }
                    }
                    // Queenside
                    if ((castleRights & WQ_CASTLE) && board[0x03] == EMPTY && board[0x02] == EMPTY && board[0x01] == EMPTY) {
                        if (!is_square_attacked(0x03, BLACK) && !is_square_attacked(0x02, BLACK)) {
                            moveList[count].from = 0x04;
                            moveList[count].to = 0x02;
                            moveList[count].captured = EMPTY;
                            moveList[count].special = 2;
                            count++;
                        }
                    }
                }
            } else {
                // Black Castling
                if (!is_square_attacked(0x74, WHITE)) {
                    // Kingside
                    if ((castleRights & BK_CASTLE) && board[0x75] == EMPTY && board[0x76] == EMPTY) {
                        if (!is_square_attacked(0x75, WHITE) && !is_square_attacked(0x76, WHITE)) {
                            moveList[count].from = 0x74;
                            moveList[count].to = 0x76;
                            moveList[count].captured = EMPTY;
                            moveList[count].special = 2;
                            count++;
                        }
                    }
                    // Queenside
                    if ((castleRights & BQ_CASTLE) && board[0x73] == EMPTY && board[0x72] == EMPTY && board[0x71] == EMPTY) {
                        if (!is_square_attacked(0x73, WHITE) && !is_square_attacked(0x72, WHITE)) {
                            moveList[count].from = 0x74;
                            moveList[count].to = 0x72;
                            moveList[count].captured = EMPTY;
                            moveList[count].special = 2;
                            count++;
                        }
                    }
                }
            }
        } else {                // Knight, Bishop, Rook, Queen
            const BYTE *offsets = (type == KNIGHT) ? knight_offsets : (type == ROOK) ? rook_offsets : (type == BISHOP) ? bishop_offsets : NULL;
            BYTE len = (type == KNIGHT) ? 8 : 4;

            // Queens use both rook and bishop offsets
            if (type == QUEEN) {
                offsets = rook_offsets;
                len = 4;
            }

            // Loop for Queen (2 passes) or others (1 pass)
            BYTE passes = (type == QUEEN) ? 2 : 1;

            BYTE p;
            for (p = 0; p < passes; p++) {
                if (p == 1)
                    offsets = bishop_offsets;

                for (i = 0; i < len; i++) {
                    offset = offsets[i];
                    target_sq = sq + offset;

                    if (type == KNIGHT) {
                        if (IS_OFFBOARD(target_sq))
                            continue;
                        UBYTE target = board[target_sq];
                        if (target == EMPTY || GET_COLOR(target) != color) {
                            moveList[count].from = sq;
                            moveList[count].to = target_sq;
                            moveList[count].captured = target;
                            moveList[count].special = 0;
                            count++;
                        }
                    } else {    // Sliders
                        while (!IS_OFFBOARD(target_sq)) {
                            UBYTE target = board[target_sq];
                            if (target == EMPTY) {
                                moveList[count].from = sq;
                                moveList[count].to = target_sq;
                                moveList[count].captured = EMPTY;
                                moveList[count].special = 0;
                                count++;
                            } else {
                                if (GET_COLOR(target) != color) {
                                    moveList[count].from = sq;
                                    moveList[count].to = target_sq;
                                    moveList[count].captured = target;
                                    moveList[count].special = 0;
                                    count++;
                                }
                                break;
                            }
                            target_sq += offset;
                        }
                    }
                }
            }
        }
    }
    return count;
}

// Returns 0 if legal, 1 if illegal (leaves King in check)
UBYTE make_move(Move *m) {
    UBYTE from = m->from;
    UBYTE to = m->to;
    UBYTE piece = board[from];
    UBYTE type = GET_TYPE(piece);
    UBYTE color = GET_COLOR(piece);

    // Save state
    m->old_castle = castleRights;
    m->old_ep = epSquare;

    // Move Piece
    board[to] = piece;
    board[from] = EMPTY;

    // Special Moves
    if (m->special == 1) {      // EP
        UBYTE cap_sq = (color == WHITE) ? to - 16 : to + 16;
        board[cap_sq] = EMPTY;
    } else if (m->special == 2) {       // Castling
        if (to == 0x06) {
            board[0x05] = board[0x07];
            board[0x07] = EMPTY;
        } else if (to == 0x02) {
            board[0x03] = board[0x00];
            board[0x00] = EMPTY;
        } else if (to == 0x76) {
            board[0x75] = board[0x77];
            board[0x77] = EMPTY;
        } else if (to == 0x72) {
            board[0x73] = board[0x70];
            board[0x70] = EMPTY;
        }
    }

    // Update Rights
    if (type == KING) {
        if (color == WHITE)
            castleRights &= ~(WK_CASTLE | WQ_CASTLE);
        else
            castleRights &= ~(BK_CASTLE | BQ_CASTLE);
    }
    if (from == 0x00 || to == 0x00)
        castleRights &= ~WQ_CASTLE;
    if (from == 0x07 || to == 0x07)
        castleRights &= ~WK_CASTLE;
    if (from == 0x70 || to == 0x70)
        castleRights &= ~BQ_CASTLE;
    if (from == 0x77 || to == 0x77)
        castleRights &= ~BK_CASTLE;

    epSquare = 0;
    if (type == PAWN) {
        int diff = (int) to - (int) from;       // Use int for safety
        if (diff == 32 || diff == -32)
            epSquare = (from + to) / 2;
    }

    // Check Legality (Crucial Step)
    UBYTE myKingSq = 0;
    UBYTE myKing = color | KING;

    // Optimization: If the piece moved IS the King, we know where it is.
    if (type == KING) {
        myKingSq = to;
    } else {
        // Otherwise, scan for it.
        // We scan specifically for 'myKing' which we constructed cleanly above.
        for (UBYTE i = 0; i < BOARD_SIZE; i++) {
            if (!IS_OFFBOARD(i) && board[i] == myKing) {
                myKingSq = i;
                break;
            }
        }
    }

    // Check if attacked by OPPONENT
    if (is_square_attacked(myKingSq, OPPONENT(color))) {
        sideToMove = OPPONENT(sideToMove);      // Ensure side is flipped even if returning illegal
        return 1;               // Illegal
    }

    sideToMove = OPPONENT(sideToMove);
    return 0;                   // Legal
}

void unmake_move(Move *m) {
    sideToMove = OPPONENT(sideToMove);  // Swap back

    // Restore board
    UBYTE piece = board[m->to];
    board[m->from] = piece;
    board[m->to] = m->captured;

    // Handle Special Unmakes
    if (m->special == 1) {      // Restore EP captured pawn
        UBYTE cap_sq = (sideToMove == WHITE) ? m->to - 16 : m->to + 16;
        board[cap_sq] = OPPONENT(sideToMove) | PAWN;
        board[m->to] = EMPTY;
    } else if (m->special == 2) {       // Undo Castling Rook
        if (m->to == 0x06) {
            board[0x07] = board[0x05];
            board[0x05] = EMPTY;
        } else if (m->to == 0x02) {
            board[0x00] = board[0x03];
            board[0x03] = EMPTY;
        } else if (m->to == 0x76) {
            board[0x77] = board[0x75];
            board[0x75] = EMPTY;
        } else if (m->to == 0x72) {
            board[0x70] = board[0x73];
            board[0x73] = EMPTY;
        }
    }

    // Restore State
    castleRights = m->old_castle;
    epSquare = m->old_ep;
}

// --- Test Logic ---

uint32_t nodes;

void perft(UBYTE depth) {
    if (depth == 0) {
        nodes++;
        return;
    }

    Move moveList[MAX_MOVES];
    UBYTE i;
    UBYTE count = generate_moves(moveList);

    for (i = 0; i < count; i++) {
        if (make_move(&moveList[i]) == 0) {     // Only recurse if legal
            perft(depth - 1);
            unmake_move(&moveList[i]);
        } else {
            // If illegal, we MUST unmake it to restore board state
            // (make_move modifies board even if it returns 1)
            unmake_move(&moveList[i]);
        }
    }
}

void run_perft_test(UBYTE depth) {
    nodes = 0;
    printf("Test Depth: %d... ", (int) depth);
#ifndef __TARGET_gb
    fflush(stdout);             // Ensure print before computation
#endif
    perft(depth);
    printf("Nodes: %" PRIu32 "\n", nodes);
}

int main(void) {
    init_board();

    printf("Chess Engine - Validator\n");

    run_perft_test(1);          // 20
    run_perft_test(2);          // 400
    run_perft_test(3);          // 8,902
    run_perft_test(4);          // 197,281
    run_perft_test(5);          // 4,865,609

    // Depth 6 on PC only (takes too long on GB)
#ifndef __TARGET_gb
    run_perft_test(6);          // 119,060,324
#endif

#ifdef __TARGET_gb
    while (1) {
    }
#else
    return 0;
#endif
}
