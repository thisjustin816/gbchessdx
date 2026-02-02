#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "chess.h"
#include "eval.h"
#include "engine.h"

#ifdef __TARGET_gb
#include <gb/gb.h>
#include <gbdk/console.h>
#include <rand.h>
#else
#include <time.h>
#endif

// A full AI
void play_full_game(void) {
    uint8_t ply = 0;

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
#ifdef __TARGET_gb
    cls();
#endif

    while (ply < 200) {
        Move best = search_best_move(3);        // Depth 3 (Main) + Quiescence

        if (best.special == 255) {
            printf("\n--- GAME OVER ---\n");
            if (checkmate_or_stalemate()) {
                printf("CHECKMATE! %s Wins.\n", (sideToMove == WHITE) ? "Black" : "White");
            } else {
                printf("STALEMATE! It's a draw.\n");
            }
            return;
        }

        make_move(&best);

#ifdef __TARGET_gb
        gotoxy(0, 0);
#endif
        printf("Ply %d: Moving %d->%d (Score: %d)\n", ply + 1, best.from, best.to, gameScore);
        print_board();

        ply++;

#ifndef __TARGET_gb
        // system("sleep 1"); 
#endif
    }

    printf("Game stopped (Max ply reached)\n");
}

int main(void) {
    play_full_game();
    return 0;
}
