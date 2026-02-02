#ifndef ENGINE_H
#define ENGINE_H

// Limits
#define MAX_SEARCH_DEPTH 6      // Enough for depth 3 + 3 Quiescence
#define MAX_PLY_MOVES MAX_MOVES // Reduce max moves considered per ply to save RAM (rarely >64 legal moves)
#define STACK_SIZE (MAX_SEARCH_DEPTH * MAX_PLY_MOVES)

#define INFINITY 20000
#define MATE_SCORE 10000

// Main Interface
// Returns the best move found for the current 'sideToMove'
Move search_best_move(uint8_t depth);

#endif
