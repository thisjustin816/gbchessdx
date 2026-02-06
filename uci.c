/* uci.c - A minimal UCI interface for testing */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include "chess.h"
#include "eval.h"
#include "engine.h"
#include "book.h"

void log_debug(const char *fmt, ...) {
    FILE *f = fopen("debug.log", "a");  // Append mode
    if (!f)
        return;

    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);

    fprintf(f, "\n");           // Auto-newline
    fclose(f);                  // Close immediately so we don't lose data if it crashes
}

// Helper: Parse "e2e4" into a Move
int parse_move_string(char *str, Move *m) {
    int f_col = str[0] - 'a';
    int f_row = str[1] - '1';
    int t_col = str[2] - 'a';
    int t_row = str[3] - '1';
    int from = (f_row * 16) + f_col;
    int to = (t_row * 16) + t_col;

    // We must validate to get captured/special flags
    Move moves[MAX_MOVES];
    uint8_t count = generate_moves(moves);
    for (int i = 0; i < count; i++) {
        if (moves[i].from == from && moves[i].to == to) {
            // Check for Promotion (string has 5th char)
            char p = str[4];

            // If the UCI string has a promotion char (q, r, b, n)
            if (p == 'q' || p == 'r' || p == 'b' || p == 'n') {
                // And the generated move is a promotion
                if (moves[i].special == 3) {
                    *m = moves[i];
                    return 1;
                }
            }
            // If the UCI string is NOT a promotion, pick the normal move
            if (moves[i].special != 3) {
                *m = moves[i];
                return 1;
            }
        }
    }
    return 0;
}

void uci_loop() {
    char line[64 * 1024];
    char move[256];
    printf("id name GameBoyChess\n");
    printf("id author Nick Craig-Wood\n");
    printf("uciok\n");

    while (fgets(line, sizeof(line), stdin)) {
        size_t length = strlen(line);
        /* if (length < 1) */
        /*     continue; */
        /* if (line[length-1] == '\n') { */
        /*     length--; */
        /*     line[length-1] = 0; */
        /* } */
        if (length < 2)
            continue;
        log_debug("Received command: '%s'", line);

        if (strncmp(line, "isready", 7) == 0) {
            printf("readyok\n");
        } else if (strncmp(line, "position startpos moves", 23) == 0) {
            init_board();
            init_game_history();        // Reset history

            char *ptr = line + 24;      // Skip "position startpos moves "
            char *token = strtok(ptr, " \n");
            while (token) {
                Move m;
                if (parse_move_string(token, &m)) {
                    make_move(&m);
                    record_history();
                }
                token = strtok(NULL, " \n");
            }
        } else if (strncmp(line, "position startpos", 17) == 0) {
            init_board();       // Just reset if no moves follow
            init_game_history();
        } else if (strncmp(line, "go", 2) == 0) {
            log_debug("Received GO command");

            // Parse depth if provided, else default to 3
            int depth = 3;

            // Check for "depth" parameter
            char *ptr = strstr(line, "depth");
            if (ptr) {
                // "depth" is 5 characters long. 
                // We advance the pointer by 5 to point to the text after "depth".
                // atoi() automatically skips any spaces before the number.
                int d = atoi(ptr + 5);
                if (d > 0)
                    depth = d;
            }
            log_debug("using depth %d\n", depth);

            // Find best move at level
            Move best = computer_move((uint8_t) depth);

            // Output in UCI format (Long Algebraic)
            const char *promotion = (best.special == 3) ? "q" : "";
            sprintf(move, "bestmove %c%d%c%d%s\n", (best.from & 7) + 'a', (best.from >> 4) + 1, (best.to & 7) + 'a', (best.to >> 4) + 1, promotion);
            printf("%s", move);
            log_debug(move);

            fflush(stdout);
        } else if (strncmp(line, "quit", 4) == 0) {
            break;
        }
    }
}

int main() {
    // Disable buffering on stdin and stdout
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    // Standard Init
    uci_loop();
    return 0;
}
