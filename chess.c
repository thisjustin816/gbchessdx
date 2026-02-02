/* * 0x88 Chess Engine
 * Compatible with GBDK-2000 and Standard C
 */

#include <stdint.h>
#include <stdio.h>
#include "chess.h"
#include "eval.h"

// Global State
uint8_t board[BOARD_SIZE];
uint8_t sideToMove = WHITE;
uint8_t castleRights = 0;       // Bitmask: 0000 KQkq
uint8_t epSquare = 0;           // 0 if no EP, otherwise the square behind the pawn

// --- Lookup Data ---
const int8_t knight_offsets[] = { -33, -31, -18, -14, 14, 18, 31, 33 };
const int8_t king_offsets[] = { -17, -16, -15, -1, 1, 15, 16, 17 };
const int8_t bishop_offsets[] = { -17, -15, 15, 17 };
const int8_t rook_offsets[] = { -16, -1, 1, 16 };

// --- Helper Functions ---

// Check if a specific square is attacked by 'side'
uint8_t is_square_attacked(uint8_t sq, uint8_t side) {
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
    uint8_t i;
    for (i = 0; i < 8; i++) {
        uint8_t target = sq - knight_offsets[i];        // Reverse offset check
        if (!IS_OFFBOARD(target)) {
            if (board[target] == (side | KNIGHT))
                return 1;
        }
    }

    // Check Kings
    for (i = 0; i < 8; i++) {
        uint8_t target = sq - king_offsets[i];
        if (!IS_OFFBOARD(target)) {
            if (board[target] == (side | KING))
                return 1;
        }
    }

    // Check Sliders (Rook/Queen)
    for (i = 0; i < 4; i++) {
        int8_t offset = rook_offsets[i];
        uint8_t target = sq + offset;
        while (!IS_OFFBOARD(target)) {
            uint8_t piece = board[target];
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
        int8_t offset = bishop_offsets[i];
        uint8_t target = sq + offset;
        while (!IS_OFFBOARD(target)) {
            uint8_t piece = board[target];
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
    uint8_t i;
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

    init_eval();                // Initialize Evaluation
}

// --- Move Generation & Execution ---

uint8_t generate_moves(Move *moveList) {
    uint8_t count = 0;
    uint8_t sq, piece, type, color;
    int8_t i, offset;
    uint8_t target_sq;

    for (sq = 0; sq < BOARD_SIZE; sq++) {
        if (count >= MAX_MOVES)
            return count;

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
            int8_t dir = (color == WHITE) ? 16 : -16;
            uint8_t start_rank = (color == WHITE) ? 1 : 6;

            // Forward 1
            if (!IS_OFFBOARD(sq + dir) && board[sq + dir] == EMPTY) {
                moveList[count].from = sq;
                moveList[count].to = sq + dir;
                moveList[count].captured = EMPTY;
                // Detect Promotion
                uint8_t dest_rank = (sq + dir) >> 4;
                if (dest_rank == 0 || dest_rank == 7) {
                    moveList[count].special = 3;        // 3 = Promotion
                } else {
                    moveList[count].special = 0;
                }
                count++;
                if (count >= MAX_MOVES)
                    return count;

                // Forward 2
                if ((sq >> 4) == start_rank && board[sq + dir + dir] == EMPTY) {
                    moveList[count].from = sq;
                    moveList[count].to = sq + dir + dir;
                    moveList[count].captured = EMPTY;
                    moveList[count].special = 0;        // EP flag set in make_move logic via distance
                    count++;
                    if (count >= MAX_MOVES)
                        return count;
                }
            }
            // Captures (Normal + En Passant)
            int8_t cap_offsets[] = { 15, 17 };
            for (i = 0; i < 2; i++) {
                offset = (color == WHITE) ? cap_offsets[i] : -cap_offsets[i];
                target_sq = sq + offset;
                if (IS_OFFBOARD(target_sq))
                    continue;

                uint8_t target = board[target_sq];
                if (target != EMPTY && GET_COLOR(target) != color) {
                    moveList[count].from = sq;
                    moveList[count].to = target_sq;
                    moveList[count].captured = target;
                    // Detect Promotion
                    uint8_t dest_rank = target_sq >> 4;
                    if (dest_rank == 0 || dest_rank == 7) {
                        moveList[count].special = 3;    // 3 = Promotion
                    } else {
                        moveList[count].special = 0;
                    }
                    count++;
                    if (count >= MAX_MOVES)
                        return count;
                }
                // En Passant
                else if (target_sq == epSquare && epSquare != 0) {
                    moveList[count].from = sq;
                    moveList[count].to = target_sq;
                    moveList[count].captured = EMPTY;   // Capture logic handled specially
                    moveList[count].special = 1;        // Mark as EP
                    count++;
                    if (count >= MAX_MOVES)
                        return count;
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
                    if (count >= MAX_MOVES)
                        return count;
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
                            if (count >= MAX_MOVES)
                                return count;
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
                            if (count >= MAX_MOVES)
                                return count;
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
                            if (count >= MAX_MOVES)
                                return count;
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
                            if (count >= MAX_MOVES)
                                return count;
                        }
                    }
                }
            }
        } else {                // Knight, Bishop, Rook, Queen
            const int8_t *offsets = (type == KNIGHT) ? knight_offsets : (type == ROOK) ? rook_offsets : (type == BISHOP) ? bishop_offsets : NULL;
            int8_t len = (type == KNIGHT) ? 8 : 4;

            // Queens use both rook and bishop offsets
            if (type == QUEEN) {
                offsets = rook_offsets;
                len = 4;
            }
            // Loop for Queen (2 passes) or others (1 pass)
            int8_t passes = (type == QUEEN) ? 2 : 1;

            int8_t p;
            for (p = 0; p < passes; p++) {
                if (p == 1)
                    offsets = bishop_offsets;

                for (i = 0; i < len; i++) {
                    offset = offsets[i];
                    target_sq = sq + offset;

                    if (type == KNIGHT) {
                        if (IS_OFFBOARD(target_sq))
                            continue;
                        uint8_t target = board[target_sq];
                        if (target == EMPTY || GET_COLOR(target) != color) {
                            moveList[count].from = sq;
                            moveList[count].to = target_sq;
                            moveList[count].captured = target;
                            moveList[count].special = 0;
                            count++;
                            if (count >= MAX_MOVES)
                                return count;
                        }
                    } else {    // Sliders
                        while (!IS_OFFBOARD(target_sq)) {
                            uint8_t target = board[target_sq];
                            if (target == EMPTY) {
                                moveList[count].from = sq;
                                moveList[count].to = target_sq;
                                moveList[count].captured = EMPTY;
                                moveList[count].special = 0;
                                count++;
                                if (count >= MAX_MOVES)
                                    return count;
                            } else {
                                if (GET_COLOR(target) != color) {
                                    moveList[count].from = sq;
                                    moveList[count].to = target_sq;
                                    moveList[count].captured = target;
                                    moveList[count].special = 0;
                                    count++;
                                    if (count >= MAX_MOVES)
                                        return count;
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
uint8_t make_move(Move *m) {
    uint8_t from = m->from;
    uint8_t to = m->to;
    uint8_t piece = board[from];
    uint8_t type = GET_TYPE(piece);
    uint8_t color = GET_COLOR(piece);

    // Save state
    m->old_castle = castleRights;
    m->old_ep = epSquare;

    // Remove moving piece from 'from'
    eval_remove_piece(piece, from);

    // If capture, remove captured piece from 'to'
    if (m->captured != EMPTY) {
        eval_remove_piece(m->captured, to);
    }
    // Move Piece
    board[to] = piece;
    board[from] = EMPTY;

    // Add moving piece to 'to'
    eval_add_piece(piece, to);

    // Special Moves
    if (m->special == 1) {      // EP
        uint8_t cap_sq = (color == WHITE) ? to - 16 : to + 16;
        uint8_t captured_pawn = OPPONENT(color) | PAWN;
        board[cap_sq] = EMPTY;

        // Remove EP captured pawn
        eval_remove_piece(captured_pawn, cap_sq);

    } else if (m->special == 2) {       // Castling
        // Move Rook
        uint8_t rook = color | ROOK;
        if (to == 0x06) {       // WK Side
            board[0x05] = board[0x07];
            board[0x07] = EMPTY;
            eval_move_piece(rook, 0x07, 0x05);
        } else if (to == 0x02) {        // WQ Side
            board[0x03] = board[0x00];
            board[0x00] = EMPTY;
            eval_move_piece(rook, 0x00, 0x03);
        } else if (to == 0x76) {        // BK Side
            board[0x75] = board[0x77];
            board[0x77] = EMPTY;
            eval_move_piece(rook, 0x77, 0x75);
        } else if (to == 0x72) {        // BQ Side
            board[0x73] = board[0x70];
            board[0x70] = EMPTY;
            eval_move_piece(rook, 0x70, 0x73);
        }
    } else if (m->special == 3) {       // Promotion
        // The pawn has already moved to 'to' by the standard logic above.
        // Now transform it into a Queen.
        uint8_t queen = color | QUEEN;
        board[to] = queen;

        // Update Eval: Remove the Pawn value we just added, add Queen value instead
        eval_remove_piece(color | PAWN, to);
        eval_add_piece(queen, to);
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
        int diff = (int) to - (int) from;
        if (diff == 32 || diff == -32)
            epSquare = (from + to) / 2;
    }
    // ALWAYS Flip side
    sideToMove = OPPONENT(sideToMove);

    // Check Legality
    uint8_t myKingSq = 0;
    uint8_t myKing = color | KING;

    if (type == KING) {
        myKingSq = to;
    } else {
        for (uint8_t i = 0; i < BOARD_SIZE; i++) {
            if (!IS_OFFBOARD(i) && board[i] == myKing) {
                myKingSq = i;
                break;
            }
        }
    }

    // Check if attacked by OPPONENT
    // Note: 'sideToMove' is now the opponent, so we can use it or OPPONENT(color)
    if (is_square_attacked(myKingSq, OPPONENT(color))) {
        return 1;               // Illegal (Side is already flipped, so unmake will fix it)
    }

    return 0;                   // Legal
}

void unmake_move(Move *m) {
    sideToMove = OPPONENT(sideToMove);  // Swap back

    uint8_t piece = board[m->to];       // The piece that moved

    // Reverse move of the piece
    eval_remove_piece(piece, m->to);
    eval_add_piece(piece, m->from);

    // Restore board
    board[m->from] = piece;
    board[m->to] = m->captured;

    // Restore captured piece
    if (m->captured != EMPTY) {
        eval_add_piece(m->captured, m->to);
    }
    // Handle Special Unmakes
    if (m->special == 1) {      // Restore EP captured pawn
        uint8_t cap_sq = (sideToMove == WHITE) ? m->to - 16 : m->to + 16;
        uint8_t captured_pawn = OPPONENT(sideToMove) | PAWN;
        board[cap_sq] = captured_pawn;
        board[m->to] = EMPTY;

        // Restore EP captured pawn
        eval_add_piece(captured_pawn, cap_sq);

    } else if (m->special == 2) {       // Undo Castling Rook
        uint8_t rook = sideToMove | ROOK;
        if (m->to == 0x06) {
            board[0x07] = board[0x05];
            board[0x05] = EMPTY;
            eval_move_piece(rook, 0x05, 0x07);
        } else if (m->to == 0x02) {
            board[0x00] = board[0x03];
            board[0x03] = EMPTY;
            eval_move_piece(rook, 0x03, 0x00);
        } else if (m->to == 0x76) {
            board[0x77] = board[0x75];
            board[0x75] = EMPTY;
            eval_move_piece(rook, 0x75, 0x77);
        } else if (m->to == 0x72) {
            board[0x70] = board[0x73];
            board[0x73] = EMPTY;
            eval_move_piece(rook, 0x73, 0x70);
        }
    } else if (m->special == 3) {       // Undo Promotion
        // The standard logic above moved the piece back from 'to' to 'from'.
        // But that piece was a Queen. We need to turn it back into a Pawn.

        uint8_t pawn = sideToMove | PAWN;       // sideToMove is already flipped back
        board[m->from] = pawn;

        // Update Eval: Remove the Queen, add the Pawn
        eval_remove_piece(sideToMove | QUEEN, m->from);
        eval_add_piece(pawn, m->from);
    }
    // Restore State
    castleRights = m->old_castle;
    epSquare = m->old_ep;
}

// Returns 1 if it was checkmate, 0 if stalemate.
// Call only when no legal moves
uint8_t checkmate_or_stalemate(void) {
    // Determine if Checkmate or Stalemate
    uint8_t myKingSq = 0;
    uint8_t myKing = sideToMove | KING;

    // Find King
    for (uint8_t i = 0; i < BOARD_SIZE; i++) {
        if (!IS_OFFBOARD(i) && board[i] == myKing) {
            myKingSq = i;
            break;
        }
    }

    if (is_square_attacked(myKingSq, OPPONENT(sideToMove))) {
        return 1;
    }
    return 0;
}

void print_board(void) {
    int8_t r, c;

    printf("\n");
    // Loop rows from 8 down to 1 (Rank 7 down to 0 in 0x88 math)
    for (r = 7; r >= 0; r--) {
        printf(" %d  ", r + 1); // Rank number

        for (c = 0; c < 8; c++) {
            uint8_t sq = (r * 16) + c;
            uint8_t piece = board[sq];
            char symbol = '.';

            if (piece != EMPTY) {
                uint8_t type = GET_TYPE(piece);
                uint8_t color = GET_COLOR(piece);

                // Base symbols for White
                const char symbols[] = "?PNBRQK";
                symbol = symbols[type];

                // If Black, convert to lowercase (ASCII math)
                if (color == BLACK) {
                    symbol += 32;
                }
            }
            if (c == 7) {
                printf("%c\n", symbol);
            } else {
                printf("%c ", symbol);
            }
        }
    }
    printf("\n    a b c d e f g h\n");
    printf("%s Eval: %6d\n", (sideToMove == WHITE) ? "White" : "Black", gameScore);
}
