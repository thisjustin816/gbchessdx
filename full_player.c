#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "chess.h"
#include "eval.h"
#include "engine.h"
#include "book.h"

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
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    unsigned int seed = (unsigned int) (ts.tv_nsec);
    printf("Seed = %u\n", seed);
    srand(seed);
#endif

    init_board();
    init_game_history();
    print_board();
#ifdef __TARGET_gb
    cls();
#endif

    while (ply < 200) {
        Move best = computer_move(3);

        make_move(&best);
        record_history();
        // Record it in permanent history

#ifdef __TARGET_gb
        gotoxy(0, 0);
#endif
        printf("Ply %d: Moving %d->%d (Score: %d)\n", ply + 1, best.from, best.to, gameScore);
        print_board();

        ply++;

        if (game_over()) {
            if (checkmate_or_stalemate()) {
                printf("Checkmate for ");
                if (sideToMove == WHITE) {
                    printf("Black\n");
                } else {
                    printf("White\n");
                }
            } else {
                printf("Stalemate: Draw\n");
            }
            return;
        } else if (is_check()) {
            printf("Check\n");
        }
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
