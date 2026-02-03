#ifndef ENGINE_H
#define ENGINE_H

// Limits
#define MAX_SEARCH_DEPTH 6      // Enough for depth 3 + 3 Quiescence
#define MAX_PLY_MOVES MAX_MOVES // Reduce max moves considered per ply to save RAM (rarely >64 legal moves)
#define STACK_SIZE (MAX_SEARCH_DEPTH * MAX_PLY_MOVES)

#define INFINITY 20000
#define MATE_SCORE 10000

// Penalty for drawing. 
// 20 centipawns means "I would rather be slightly worse (-19) than draw (-20)"
#define CONTEMPT_FACTOR 20

// Main Interface
// Returns the best move found for the current 'sideToMove'
Move search_best_move(uint8_t depth);

// Call this before the game loop starts
void init_game_history(void);

// Call this after doing a move
void record_history(void);

// Ply of game we are on
extern uint8_t game_ply;

#endif
