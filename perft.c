#include <stdint.h>
#include <stdio.h>
#include "chess.h"

#ifdef __TARGET_gb
#include <gb/gb.h>
#define PRIu32 "u"
#else
#include <inttypes.h>
#endif

uint32_t nodes;

uint32_t expected[] = {
    1,
    20,
    400,
    8902,
    197281,
    4865609,
    119060324,
};

void perft(uint8_t depth) {
    if (depth == 0) {
        nodes++;
        return;
    }

    Move moveList[MAX_MOVES];
    uint8_t i;
    uint8_t count = generate_moves(moveList);

    for (i = 0; i < count; i++) {
        if (make_move(&moveList[i]) == 0) {     // Only recurse if legal
            perft(depth - 1);
            unmake_move(&moveList[i]);
        } else {
            // If illegal, we MUST unmake it to restore board state
            // (make_move modifies board even if it returns 1)
            unmake_move(&moveList[i]);
        }
    }
}

void run_perft_test(uint8_t depth) {
    nodes = 0;
    printf("Test Depth: %d... ", (int) depth);
#ifndef __TARGET_gb
    fflush(stdout);             // Ensure print before computation
#endif
    perft(depth);
    printf("Nodes: %" PRIu32, nodes);
    if (expected[depth] == nodes) {
        printf(" OK\n");
    } else {
        printf(" FAIL want %" PRIu32 "\n", expected[depth]);
    }
}

int main(void) {
    init_board();

    printf("Chess Engine - Validator\n");

    for (uint8_t i = 1; i <= 6; i++) {
        run_perft_test(i);
    }

#ifdef __TARGET_gb
    while (1) {
    }
#else
    return 0;
#endif
}
