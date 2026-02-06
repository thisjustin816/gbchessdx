#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "chess.h"
#include "engine.h"

#ifdef __TARGET_gb
#include <gb/gb.h>
#include <gbdk/console.h>
#include <rand.h>
#else
#include <time.h>
#endif

void play_random_game(void) {
    uint8_t ply = 0;

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

    while (ply < 200) {         // Limit game length to avoid infinite loops
        Move selectedMove = get_random_move();

        // Execute and Print
        make_move(&selectedMove);
#ifdef __TARGET_gb
        gotoxy(0, 0);
#endif
        printf("Ply %d: Moving from %d to %d\n", ply + 1, selectedMove.from, selectedMove.to);
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
    }

    printf("Game stopped (Max ply reached)\n");
}

int main(void) {
    play_random_game();
    return 0;
}
