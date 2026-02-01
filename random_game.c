#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "chess.h"

#ifdef __TARGET_gb
#include <gb/gb.h>
#include <rand.h>
#else
#include <time.h>
#endif

void play_random_game(void) {
    uint8_t ply = 0;
    uint8_t i;

    // Seed random number generator
#ifdef __TARGET_gb
    uint16_t seed = DIV_REG;
    seed |= (uint16_t) LY_REG << 8;
    initrand(seed);
#else
    srand(time(NULL));
#endif

    init_board();
    print_board();

    while (ply < 200) {         // Limit game length to avoid infinite loops
        Move moveList[MAX_MOVES];
        uint8_t all_moves_count = generate_moves(moveList);

        // We need to store indices of strictly LEGAL moves
        uint8_t legal_indices[MAX_MOVES];
        uint8_t legal_count = 0;

        // Filter moves: Try each one to see if it's legal
        for (i = 0; i < all_moves_count; i++) {
            if (make_move(&moveList[i]) == 0) {
                // It was legal!
                legal_indices[legal_count++] = i;
            }
            unmake_move(&moveList[i]);
        }

        // Check Game Over Conditions
        if (legal_count == 0) {
            printf("\n--- GAME OVER ---\n");

            if (checkmate_or_stalemate()) {
                printf("CHECKMATE! %s Wins.\n", (sideToMove == WHITE) ? "Black" : "White");
            } else {
                printf("STALEMATE! It's a draw.\n");
            }
            return;
        }
        // Pick a Random Legal Move
        // rand() % N returns 0 to N-1
        int rand_index = rand() % legal_count;
        Move *selectedMove = &moveList[legal_indices[rand_index]];

        // Execute and Print
        make_move(selectedMove);
        printf("Ply %d: Moving from %d to %d\n", ply + 1, selectedMove->from, selectedMove->to);
        print_board();

        ply++;

        // Optional: Add a delay on PC so you can watch it
        // system("sleep 1"); // Linux
    }

    printf("Game stopped (Max ply reached)\n");
}

int main(void) {
    play_random_game();
    return 0;
}
