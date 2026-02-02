#include <stdint.h>
#include "chess.h"
#include "eval.h"
#include "engine.h"

// --- Global Search Memory ---
// We use a global stack to avoid blowing the CPU stack with recursion.
Move global_move_stack[STACK_SIZE];

// Helper to calculate MVV/LVA score for move ordering
// (Most Valuable Victim - Least Valuable Aggressor)
// E.g., Pawn captures Queen is huge. Queen captures Pawn is small.
int16_t score_move_ordering(Move *m) {
    if (m->captured != EMPTY) {
        // Victim Value: 10 * Piece Value (to prioritize capturing big things)
        // Aggressor Value: Subtract Piece Value (to prioritize risking small things)
        // Values: P=100, N=320, B=330, R=500, Q=900, K=20000

        int16_t victim = material_value[GET_TYPE(m->captured)];
        int16_t aggressor = material_value[GET_TYPE(board[m->from])];   // Note: board not moved yet

        return (victim * 10) - aggressor;
    }
    // TODO: Add History Heuristic or PST bonus here for non-captures
    return 0;
}

// Selection Sort: Find the best move in the list from 'startIndex' to 'count'
// and swap it to 'startIndex'. 
void pick_next_move(uint8_t moveNum, uint8_t count, Move *basePtr) {
    int16_t bestScore = -32000;
    uint8_t bestIndex = moveNum;

    // Scan remaining moves
    for (uint8_t i = moveNum; i < count; i++) {
        int16_t score = score_move_ordering(&basePtr[i]);
        if (score > bestScore) {
            bestScore = score;
            bestIndex = i;
        }
    }

    // Swap best to front
    if (bestIndex != moveNum) {
        Move temp = basePtr[moveNum];
        basePtr[moveNum] = basePtr[bestIndex];
        basePtr[bestIndex] = temp;
    }
}

// --- Quiescence Search ---
// Only searches captures to stabilize the evaluation (avoids horizon effect)
int16_t quiescence(int16_t alpha, int16_t beta, uint16_t ply_offset) {
    // Stand Pat: If we do nothing, what is the score?
    // We assume the side to move can at least choose "not to move" if they are ahead.
    // (Note: sideToMove is managed by make_move, so eval_score is already relative to board state?
    // No, gameScore is global White-Black. We need to flip it for the current player.)

    int16_t stand_pat = (sideToMove == WHITE) ? gameScore : -gameScore;

    if (stand_pat >= beta)
        return beta;
    if (alpha < stand_pat)
        alpha = stand_pat;

    // Safety check for memory
    if (ply_offset >= STACK_SIZE - MAX_PLY_MOVES)
        return stand_pat;

    // Generate Moves (Captures only ideally, but we filter below)
    Move *plyMoves = &global_move_stack[ply_offset];

    // Optimization: In a real engine we'd write a separate generate_captures()
    // Here we generate all and filter.
    uint8_t count = generate_moves(plyMoves);

    for (uint8_t i = 0; i < count; i++) {
        // Pick best move first (Move Ordering)
        pick_next_move(i, count, plyMoves);
        Move *m = &plyMoves[i];

        // QS Optimization: Only consider CAPTURES
        if (m->captured == EMPTY)
            continue;

        // Delta Pruning (Simplified): If capturing this piece + safety margin
        // still doesn't raise score above alpha, skip it.
        // (Skipped for code size/complexity on GB for now)

        if (make_move(m) == 0) {
            // Recurse
            int16_t score = -quiescence(-beta, -alpha, ply_offset + MAX_PLY_MOVES);
            unmake_move(m);

            if (score >= beta)
                return beta;
            if (score > alpha)
                alpha = score;
        } else {
            unmake_move(m);
        }
    }
    return alpha;
}

// --- Negamax with Alpha-Beta ---
int16_t negamax(uint8_t depth, int16_t alpha, int16_t beta, uint16_t ply_offset) {
    // Leaf Node: Drop into Quiescence Search
    if (depth == 0) {
        return quiescence(alpha, beta, ply_offset);
    }
    // Safety check for memory
    if (ply_offset >= STACK_SIZE - MAX_PLY_MOVES) {
        return (sideToMove == WHITE) ? gameScore : -gameScore;
    }
    // Generate Moves
    Move *plyMoves = &global_move_stack[ply_offset];
    uint8_t count = generate_moves(plyMoves);

    // Check for Game Over (No moves)
    // We must check strict legality. This is expensive, so usually done lazily.
    // But since generate_moves generates pseudo-legal, we check inside the loop.
    // If we go through ALL moves and none are legal -> Mate/Stale.

    uint8_t legal_moves_found = 0;

    for (uint8_t i = 0; i < count; i++) {
        // Move Ordering
        pick_next_move(i, count, plyMoves);
        Move *m = &plyMoves[i];

        if (make_move(m) == 0) {
            legal_moves_found++;

            // Search
            int16_t score = -negamax(depth - 1, -beta, -alpha, ply_offset + MAX_PLY_MOVES);

            unmake_move(m);

            if (score >= beta) {
                return beta;    // Cut-off
            }
            if (score > alpha) {
                alpha = score;
            }
        } else {
            unmake_move(m);
        }
    }

    // Mate / Stalemate Detection
    if (legal_moves_found == 0) {
        if (checkmate_or_stalemate()) {
            // Checkmate: Return huge negative score (adjusted for depth to prefer faster mates)
            return -MATE_SCORE + ply_offset;    // +ply_offset favors mating sooner
        } else {
            // Stalemate
            return 0;
        }
    }

    return alpha;
}

// --- Root Search ---
Move search_best_move(uint8_t depth) {
    Move bestMove;
    // Defaults if no move found (shouldn't happen unless mated)
    bestMove.from = 0;
    bestMove.to = 0;

    int16_t alpha = -INFINITY;
    int16_t beta = INFINITY;

    Move *plyMoves = &global_move_stack[0];
    uint8_t count = generate_moves(plyMoves);
    uint8_t legal_found = 0;

    // Root loop
    for (uint8_t i = 0; i < count; i++) {
        pick_next_move(i, count, plyMoves);
        Move *m = &plyMoves[i];

        if (make_move(m) == 0) {
            legal_found = 1;

            // Call Negamax
            int16_t score = -negamax(depth - 1, -beta, -alpha, MAX_PLY_MOVES);

            unmake_move(m);

            // Simple Output for debugging
            // printf("Move %d->%d Score: %d\n", m->from, m->to, score);

            if (score > alpha) {
                alpha = score;
                bestMove = *m;
            }
        } else {
            unmake_move(m);
        }
    }

    // Handle Game Over at root
    if (legal_found == 0) {
        // Handle external logic for game over
        bestMove.special = 255; // Marker for "No Move"
    }

    return bestMove;
}
