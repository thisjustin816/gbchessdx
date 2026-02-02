#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "chess.h"
#include "eval.h"

#ifdef __TARGET_gb
#include <gb/gb.h>
#include <gbdk/console.h>
#include <rand.h>
#else
#include <time.h>
#endif

// A simple AI that looks 1 move ahead
void play_one_ply_game(void) {
    uint8_t ply = 0;
    uint8_t i;

    // Seed random number generator
#ifdef __TARGET_gb
    uint16_t seed = DIV_REG;
    seed |= (uint16_t) LY_REG << 8;
    initrand(seed);
#else
    srand((unsigned int) time(NULL));
#endif

    init_board();
    print_board();
#ifdef __TARGET_gb
    cls();
#endif

    while (ply < 200) {
        Move moveList[MAX_MOVES];
        uint8_t all_moves_count = generate_moves(moveList);

        uint8_t bestMovesIndices[MAX_MOVES];
        uint8_t bestMovesCount = 0;

        // Initialize best score to worst possible
        // If White to move, they want Max score.
        // If Black to move, they want Min score.
        int16_t bestScore = (sideToMove == WHITE) ? -30000 : 30000;

        uint8_t legal_found = 0;

        for (i = 0; i < all_moves_count; i++) {
            if (make_move(&moveList[i]) == 0) {
                legal_found = 1;

                // Move is legal, check the GLOBAL evaluation score.
                // Note: make_move has already updated 'gameScore'.
                int16_t currentEval = gameScore;

                if (OPPONENT(sideToMove) == WHITE) {
                    // It WAS White's turn, so we are maximizing for White
                    if (currentEval > bestScore) {
                        bestScore = currentEval;
                        bestMovesCount = 0;
                        bestMovesIndices[bestMovesCount++] = i;
                    } else if (currentEval == bestScore) {
                        bestMovesIndices[bestMovesCount++] = i;
                    }
                } else {
                    // It WAS Black's turn, so we are minimizing for White
                    if (currentEval < bestScore) {
                        bestScore = currentEval;
                        bestMovesCount = 0;
                        bestMovesIndices[bestMovesCount++] = i;
                    } else if (currentEval == bestScore) {
                        bestMovesIndices[bestMovesCount++] = i;
                    }
                }
            }

            unmake_move(&moveList[i]);
        }

        if (!legal_found) {
            printf("\n--- GAME OVER ---\n");
            if (checkmate_or_stalemate()) {
                printf("CHECKMATE! %s Wins.\n", (sideToMove == WHITE) ? "Black" : "White");
            } else {
                printf("STALEMATE! It's a draw.\n");
            }
            return;
        }
        // Pick a Random Best Move (if multiple moves have same best score)
        int rand_index = rand() % bestMovesCount;
        Move *selectedMove = &moveList[bestMovesIndices[rand_index]];

        make_move(selectedMove);
#ifdef __TARGET_gb
        gotoxy(0, 0);
#endif
        printf("Ply %d: Moving %d->%d (Score: %d)\n", ply + 1, selectedMove->from, selectedMove->to, gameScore);
        print_board();

        ply++;

#ifndef __TARGET_gb
        // system("sleep 1"); 
#endif
    }

    printf("Game stopped (Max ply reached)\n");
}

int main(void) {
    play_one_ply_game();
    return 0;
}
