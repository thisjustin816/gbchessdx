#ifndef BOOK_H
#define BOOK_H

typedef struct {
    uint16_t hash;
    uint8_t from;
    uint8_t to;
} BookEntry;

extern const BookEntry opening_book[];
extern const uint16_t book_len;
extern const uint8_t book_max_ply;

// Check if a move exists in the book for the current posHash.
// Returns 1 if found (and puts move in *m), 0 if not.
uint8_t get_book_move(Move * m);

#endif
