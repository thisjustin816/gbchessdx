#include <stdint.h>
#include "chess.h"
#include "engine.h"
#include "book.h"
#include "eval.h"

#ifdef __TARGET_gb
#include <rand.h>
#else
#include <stdlib.h>
#endif

uint8_t get_book_move(Move *m) {
    int low = 0;
    int high = book_len - 1;
    int index = -1;

    if (game_ply >= book_max_ply) {
        return 0;
    }
    // Binary Search to find *any* matching entry
    while (low <= high) {
        int mid = (low + high) / 2;
        if (opening_book[mid].hash < posHash) {
            low = mid + 1;
        } else if (opening_book[mid].hash > posHash) {
            high = mid - 1;
        } else {
            index = mid;
            break;              // Found one!
        }
    }

    if (index == -1)
        return 0;               // No move found

    // Randomize: There might be multiple moves for this hash.
    // Scan backward to find the first one.
    int start = index;
    while (start > 0 && opening_book[start - 1].hash == posHash) {
        start--;
    }
    // Scan forward to find the last one.
    int end = index;
    while (end < book_len - 1 && opening_book[end + 1].hash == posHash) {
        end++;
    }

    // Pick a random index between start and end
    int count = (end - start) + 1;
    int pick = start + (rand() % count);

    // Output the move
    m->from = opening_book[pick].from;
    m->to = opening_book[pick].to;

    // Critical: We must set the 'captured' flag correctly for make_move
    m->captured = board[m->to];

    // Most book moves are normal. If you have castling in your book,
    // you might need to detect it here (e.g. if king moves > 1 square).
    m->special = 0;

    // Simple Castling Detection for Book Moves:
    uint8_t piece = board[m->from];
    if (GET_TYPE(piece) == KING) {
        int diff = m->to - m->from;
        if (diff == 2 || diff == -2)
            m->special = 2;
    }

    return 1;
}
