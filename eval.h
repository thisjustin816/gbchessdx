#ifndef EVALUATOR_H
#define EVALUATOR_H

// Global Evaluation Score (Centipawns).
// Positive = White Advantage, Negative = Black Advantage.
extern int16_t gameScore;

// Helper to convert 0x88 0-127 index to 0-63 linear index
// (sq & 0xF0) >> 1 calculates row * 8
// (sq & 0x07) calculates col
#define SQ64(sq) ((uint8_t)((((sq) & 0xF0) >> 1) | ((sq) & 0x07)))

// Functions
void init_eval(void);
void eval_add_piece(uint8_t piece, uint8_t sq);
void eval_remove_piece(uint8_t piece, uint8_t sq);
void eval_move_piece(uint8_t piece, uint8_t from, uint8_t to);

// Data
extern const int16_t material_value[];
extern uint16_t posHash;        // Global Position Hash

#endif
