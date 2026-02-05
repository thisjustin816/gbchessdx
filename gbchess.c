/*
  Game Boy chess
*/

#include <gb/gb.h>
#include <rand.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#include "tiles.h"
#include "chess.h"
#include "eval.h"
#include "engine.h"
#include "book.h"
#include "sound.h"

// Tiles
#define tile_SPACE tiles_map[0xFF]
#define tile_A tiles_map[0x80]
#define tile_0 tiles_map[0x9A]
#define tilep_WHITE_ON_WHITE_0 0x00
#define tilep_WHITE_ON_WHITE_1 0x10
#define tilep_BLACK_ON_WHITE_0 0x20
#define tilep_BLACK_ON_WHITE_1 0x30
#define tilep_WHITE_ON_BLACK_0 0x40
#define tilep_WHITE_ON_BLACK_1 0x50
#define tilep_BLACK_ON_BLACK_0 0x60
#define tilep_BLACK_ON_BLACK_1 0x70
#define tile_CURSOR_0 tiles_map[0x0E]
#define tile_CURSOR_1 tiles_map[0x1E]
#define tile_CURSOR_SELECTED_0 tiles_map[0x2E]
#define tile_CURSOR_SELECTED_1 tiles_map[0x3E]
#define tile_BORDER_TL tiles_map[0xA4]
#define tile_BORDER_TM tiles_map[0xA5]
#define tile_BORDER_TR tiles_map[0xA6]
#define tile_BORDER_ML tiles_map[0xB4]
#define tile_BORDER_MR tiles_map[0xB6]
#define tile_BORDER_BL tiles_map[0xC4]
#define tile_BORDER_BM tiles_map[0xC5]
#define tile_BORDER_BR tiles_map[0xC6]

// Sprites

#define sprite_CURSOR_0 0
#define sprite_CURSOR_1 1
#define sprite_CURSOR_2 2
#define sprite_CURSOR_3 3
#define sprite_EVAL 4

// Screen Position
#define BOARD_X_START    2
#define BOARD_Y_START    1

// Screen offsets (adjusted for 16x16 sprite grid)
// Note: GBDK Sprite coordinates (0,0) are technically off-screen.
// Visible (0,0) is at (8, 16).
#define BOARD_PIXEL_X  (8 + 16) // 2 tiles right
#define BOARD_PIXEL_Y  (16 + 8) // 1 tile down

// State
#define NO_SQ 255
uint8_t selected_sq = NO_SQ;
uint8_t cursor_x = 3;
uint8_t cursor_y = 1;           // Rank 2 (White Pawn)
uint8_t prev_keys = 0;
Move next_move;

void print_xy(uint8_t x, uint8_t y, const char *s) {
    for (char c = *s++; c; c = *s++) {
        uint8_t tile = tile_SPACE;
        if (c >= 'A' && c <= 'Z') {
            tile = c - 'A' + tile_A;
        } else if (c >= 'a' && c <= 'z') {
            tile = c - 'a' + tile_A;
        } else if (c >= '0' && c <= '0') {
            tile = c - '0' + tile_0;
        }
        set_bkg_tile_xy(x, y, tile);
        x += 1;
        if (x >= 20) {
            y += 1;
            x = 0;
            if (y >= 18) {
                y = 0;
            }
        }
    }
}

// Draw the full board
void draw_board(void) {
    uint8_t screen_row, col;

    // Iterate 8 rows of the CHESS board
    for (screen_row = 0; screen_row < 8; screen_row++) {
        uint8_t board_row = 7 - screen_row;

        // We will draw 2 screen rows for every 1 chess row
        int screen_y = BOARD_Y_START + (screen_row * 2);

        // Prepare buffers for Top-Half and Bottom-Half of the row
        uint8_t line1[16];
        uint8_t line2[16];

        for (col = 0; col < 8; col++) {
            uint8_t sq_idx = (board_row * 16) + col;
            uint8_t piece = board[sq_idx];
            uint8_t piece_color = GET_COLOR(piece);

            // Is this square Light or Dark?
            // (board_row+col)%2 == 0 is Light, else Dark
            uint8_t is_dark = ((board_row + col) % 2) == 0;
            uint8_t sq_color = (is_dark ? BLACK : WHITE);

            uint8_t tl, bl;

            if (piece_color == WHITE) {
                if (is_dark) {
                    tl = tilep_WHITE_ON_BLACK_0;
                    bl = tilep_WHITE_ON_BLACK_1;
                } else {
                    tl = tilep_WHITE_ON_WHITE_0;
                    bl = tilep_WHITE_ON_WHITE_1;
                }
            } else {
                if (is_dark) {
                    tl = tilep_BLACK_ON_BLACK_0;
                    bl = tilep_BLACK_ON_BLACK_1;
                } else {
                    tl = tilep_BLACK_ON_WHITE_0;
                    bl = tilep_BLACK_ON_WHITE_1;
                }
            }

            uint8_t offset = GET_TYPE(piece) * 2;
            tl += offset;
            bl += offset;

            // Fill buffers (2 tiles wide per square)
            line1[col * 2] = tiles_map[tl];
            line1[col * 2 + 1] = tiles_map[tl + 1];
            line2[col * 2] = tiles_map[bl];
            line2[col * 2 + 1] = tiles_map[bl + 1];
        }

        // Send to VRAM
        set_bkg_tiles(BOARD_X_START, screen_y, 16, 1, line1);
        set_bkg_tiles(BOARD_X_START, screen_y + 1, 16, 1, line2);
    }
}

// Cursor shape varies with selected square
void set_cursor(void) {
    uint8_t tile = selected_sq == NO_SQ ? tile_CURSOR_0 : tile_CURSOR_SELECTED_0;
    set_sprite_tile(sprite_CURSOR_0, tile);
    set_sprite_tile(sprite_CURSOR_1, tile + 1);
    tile = selected_sq == NO_SQ ? tile_CURSOR_1 : tile_CURSOR_SELECTED_1;
    set_sprite_tile(sprite_CURSOR_2, tile);
    set_sprite_tile(sprite_CURSOR_3, tile + 1);
}

void setup_sprites(void) {
    set_cursor();
}

void update_cursor_sprite(void) {
    // Calculate Top-Left Pixel of the 16x16 Square
    uint8_t visual_row = 7 - cursor_y;

    uint8_t x = BOARD_PIXEL_X + (cursor_x * 16);
    uint8_t y = BOARD_PIXEL_Y + (visual_row * 16);

    // Position the 4 Sprites
    // Sprite 0: Top-Left
    move_sprite(0, x, y);

    // Sprite 1: Top-Right (x + 8)
    move_sprite(1, x + 8, y);

    // Sprite 2: Bottom-Left (y + 8)
    move_sprite(2, x, y + 8);

    // Sprite 3: Bottom-Right (x + 8, y + 8)
    move_sprite(3, x + 8, y + 8);
}

// Helper: Convert cursor (0-7, 0-7) to 0x88 Index
uint8_t get_cursor_sq(void) {
    return (cursor_y * 16) + cursor_x;
}

void set_selected(uint8_t i) {
    selected_sq = i;
    set_cursor();
}

// Handles making a move
// Returns the move to make or NULL
Move *handle_input(void) {
    Move *move_to_make = NULL;
    uint8_t keys = joypad();

    // "Just Pressed" logic
    static uint8_t prev_keys = 0;
    uint8_t pressed = keys & ~prev_keys;
    prev_keys = keys;

    // D-Pad Movement
    if (pressed & J_RIGHT && cursor_x < 7)
        cursor_x++;
    if (pressed & J_LEFT && cursor_x > 0)
        cursor_x--;
    if (pressed & J_UP && cursor_y < 7)
        cursor_y++;
    if (pressed & J_DOWN && cursor_y > 0)
        cursor_y--;

    // Update sprite immediately so it feels responsive
    if (pressed & (J_RIGHT | J_LEFT | J_UP | J_DOWN)) {
        update_cursor_sprite();
    }

    // Selection Logic (A Button)
    if (pressed & J_A) {
        uint8_t sq = get_cursor_sq();
        uint8_t piece = board[sq];

        // CASE A: Nothing selected yet. Try to pick up a piece.
        if (selected_sq == NO_SQ) {
            // Check if square has a piece AND it belongs to the side to move
            if (piece != EMPTY && GET_COLOR(piece) == sideToMove) {
                set_selected(sq);
                play_select_sound();
            }
        }
        // CASE B: Piece already selected. Try to move it to 'sq'.
        else {
            // If clicking the SAME square, Deselect (Put it back)
            if (sq == selected_sq) {
                set_selected(NO_SQ);
            } else {
                // Check Legality
                if (is_user_move_legal(selected_sq, sq, &next_move)) {
                    // Reset Selection
                    set_selected(NO_SQ);
                    play_select_sound();

                    // Return the move
                    move_to_make = &next_move;
                } else {
                    // Illegal move? 
                    // Option 1: Error buzz sound
                    // Option 2: If clicking another friendly piece, switch selection to that piece
                    if (piece != EMPTY && GET_COLOR(piece) == sideToMove) {
                        set_selected(sq);
                        play_select_sound();
                    } else {
                        play_error_sound();
                    }
                }
            }
        }
    }

    // B Button: Always Deselect / Cancel
    if (pressed & J_B) {
        set_selected(NO_SQ);
    }

    return move_to_make;
}

// Seed random number generator
void seed_rand(void) {
    uint16_t seed = DIV_REG;
    seed |= (uint16_t) LY_REG << 8;
    initrand(seed);
}

// Do the computer move
Move *computer_move(void) {
    // Draw a "Thinking..." icon
    print_xy(0, 17, "Thinking");

    uint8_t found_book = get_book_move(&next_move);

    if (found_book) {
        // We need to ensure 'captured' and 'special' are correct
        // because our simple book function didn't fully set them (lazy).
        next_move.captured = board[next_move.to];
        next_move.special = 0;  // Book moves are rarely En Passant/Castle (unless deep in book)
    } else {
        // Standard Search
        next_move = search_best_move(3);
    }

    // Clear "Thinking..." icon
    print_xy(0, 17, "        ");

    return &next_move;
}

// Run the game
int main(void) {
    // Load data into Background VRAM and Sprite VRAM
    // Not the most efficient but keeps things simple
    set_bkg_data(0, tiles_TILE_COUNT, tiles_tiles);
    set_sprite_data(0, tiles_TILE_COUNT, tiles_tiles);

    // Clear screen
    fill_bkg_rect(0, 0, 20, 18, tile_SPACE);

    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;

    seed_rand();
    setup_sprites();
    init_board();
    init_game_history();
    print_xy(3, 0, "Game Boy Chess");
    draw_board();
    update_cursor_sprite();
    init_sound();

    uint8_t human_to_move = true;

    while (true) {
        Move *m;
        if (human_to_move) {
            m = handle_input();
        } else {
            m = computer_move();
        }
        if (m) {
            make_move(m);
            record_history();
            draw_board();

            if (is_check()) {
                print_xy(0, 17, "Check");
            } else {
                print_xy(0, 17, "     ");
            }

            if (game_over()) {
                if (checkmate_or_stalemate()) {
                    print_xy(0, 17, "Checkmate for ");
                    if (sideToMove == WHITE) {
                        print_xy(14, 17, "Black");
                    } else {
                        print_xy(14, 17, "White");
                    }
                } else {
                    print_xy(0, 17, "Stalemate: Draw");
                }
                // FIXME wait for keypress
                return 0;
            }

            human_to_move = !human_to_move;
        }
        wait_vbl_done();
    }
}
