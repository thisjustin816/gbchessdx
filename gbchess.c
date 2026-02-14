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
#define tile_MOVE_FROM_0 tiles_map[0x4E]
#define tile_MOVE_FROM_1 tiles_map[0x5E]
#define tile_MOVE_TO_0 tiles_map[0x6E]
#define tile_MOVE_TO_1 tiles_map[0x7E]
#define tile_BORDER_TL tiles_map[0xA4]
#define tile_BORDER_H tiles_map[0xA5]
#define tile_BORDER_TR tiles_map[0xA6]
#define tile_BORDER_V tiles_map[0xB4]
#define tile_BORDER_BL tiles_map[0xC4]
#define tile_BORDER_BR tiles_map[0xC6]
#define tile_CURSOR tiles_map[0xB5]
#define tile_SPINNER_0 tiles_map[0xB0]

// Sprites

#define sprite_CURSOR_0 0
#define sprite_CURSOR_1 1
#define sprite_CURSOR_2 2
#define sprite_CURSOR_3 3
#define sprite_EVAL 4
#define sprite_FROM_0 5
#define sprite_FROM_1 6
#define sprite_FROM_2 7
#define sprite_FROM_3 8
#define sprite_TO_0 9
#define sprite_TO_1 10
#define sprite_TO_2 11
#define sprite_TO_3 12

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
uint8_t level = 1;
uint8_t initial_player = 0;
uint8_t player_to_move = 0;
uint8_t player_mapping = 2;     // bit 0 player 0, bit 1 player 1, 0 = human, 1 = computer
uint8_t main_menu_selection = 0;
uint8_t in_game_menu_selection = 0;
Move undo_move, undo_prev_move;
uint8_t undo_valid;
uint8_t spinner_state = 0;
uint16_t message_time = 0;
uint8_t busy_message = 0;

// Menus
const char *main_menu_items[] = {
    "Start Game",
    "Level",
    "Players",
    NULL
};

const char *in_game_menu_items[] = {
    "Undo",
    "Swap Sides",
    "New Game",
    "Level",
    "Players",
    "Exit",
    NULL
};

#define in_game_menu_UNDO 0
#define in_game_menu_SWAP 1
#define in_game_menu_NEW_GAME 2
#define in_game_menu_LEVEL 3
#define in_game_menu_PLAYERS 4
#define in_game_menu_EXIT 5

const char *level_menu_items[] = {
    "Beginner  900",
    "Easy     1050",
    "Medium   1250",
    "Hard     1325",
    NULL
};

const char *player_menu_items[] = {
    "Human v Human",
    "Comp  v Human",
    "Human v  Comp",
    "Comp  v  Comp",
    NULL
};


void print_xy(uint8_t x, uint8_t y, const char *s) {
    for (char c = *s++; c; c = *s++) {
        uint8_t tile = tile_SPACE;
        if (c >= 'A' && c <= 'Z') {
            tile = c - 'A' + tile_A;
        } else if (c >= 'a' && c <= 'z') {
            tile = c - 'a' + tile_A;
        } else if (c >= '0' && c <= '9') {
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

// Draw a menu in the center of the screen and let the user choose an
// item, returning its index.
//
// items should be a null terminated array of menu items.
// current_sel is a pointer to the menu index
//
// This will draw a correct sized box in the center of the screen with
// a border and allow the user to navigate with the d-pad and select
// with the A button.
void draw_menu(const char *items[], uint8_t *current_sel) {
    HIDE_SPRITES;

    uint8_t count = 0;
    uint8_t max_width = 0;

    // Calculate menu dimensions
    for (const char **ptr = items; *ptr != NULL; ptr++) {
        uint8_t len = strlen(*ptr);
        if (len > max_width)
            max_width = len;
        count++;
    }

    // Width = text + cursor space (1) + padding (1)
    uint8_t menu_w = max_width + 2;
    uint8_t menu_h = count;

    // Calculate Centered Position
    // Screen is 20x18 tiles. 
    // We add 2 to dimensions for the border.
    uint8_t x_start = (20 - (menu_w + 2)) / 2;
    uint8_t y_start = (18 - (menu_h + 2)) / 2;

    // Draw the Box

    // Draw Top Border
    set_bkg_tile_xy(x_start, y_start, tile_BORDER_TL);
    for (uint8_t i = 0; i < menu_w; i++) {
        set_bkg_tile_xy(x_start + 1 + i, y_start, tile_BORDER_H);
    }
    set_bkg_tile_xy(x_start + menu_w + 1, y_start, tile_BORDER_TR);

    // Draw Middle & Items
    for (uint8_t i = 0; i < count; i++) {
        uint8_t row_y = y_start + 1 + i;

        // Left Border
        set_bkg_tile_xy(x_start, row_y, tile_BORDER_V);

        // Clear background of the row (spaces)
        for (uint8_t j = 0; j < menu_w; j++) {
            set_bkg_tile_xy(x_start + 1 + j, row_y, tile_SPACE);
        }

        // Print the item text (offset by 2: 1 for border, 1 for cursor)
        print_xy(x_start + 2, row_y, items[i]);

        // Right Border
        set_bkg_tile_xy(x_start + menu_w + 1, row_y, tile_BORDER_V);
    }

    // Draw Bottom Border
    uint8_t y_bot = y_start + menu_h + 1;
    set_bkg_tile_xy(x_start, y_bot, tile_BORDER_BL);
    for (uint8_t i = 0; i < menu_w; i++) {
        set_bkg_tile_xy(x_start + 1 + i, y_bot, tile_BORDER_H);
    }
    set_bkg_tile_xy(x_start + menu_w + 1, y_bot, tile_BORDER_BR);

    // Input Loop
    uint8_t previous_keys = 0;
    uint8_t redraw_cursor = 1;

    while (1) {
        // Draw Cursor if needed
        if (redraw_cursor) {
            // Erase old cursors
            for (uint8_t i = 0; i < count; i++) {
                set_bkg_tile_xy(x_start + 1, y_start + 1 + i, tile_SPACE);
            }
            // Draw new cursor
            set_bkg_tile_xy(x_start + 1, y_start + 1 + *current_sel, tile_CURSOR);
            redraw_cursor = 0;
        }

        wait_vbl_done();
        uint8_t keys = joypad();

        // Handle Input (Debounced)
        // We check if the key is pressed NOW but wasn't pressed BEFORE

        if ((keys & J_UP) && !(previous_keys & J_UP)) {
            if (*current_sel > 0) {
                (*current_sel)--;
                redraw_cursor = 1;
            }
        } else if ((keys & J_DOWN) && !(previous_keys & J_DOWN)) {
            if (*current_sel < count - 1) {
                (*current_sel)++;
                redraw_cursor = 1;
            }
        } else if ((keys & J_A) && !(previous_keys & J_A)) {
            // Blink effect on selection (optional but feels nice)
            set_bkg_tile_xy(x_start + 1, y_start + 1 + *current_sel, tile_SPACE);
            delay(100);
            set_bkg_tile_xy(x_start + 1, y_start + 1 + *current_sel, tile_CURSOR);
            delay(100);
            SHOW_SPRITES;
            return;
        }

        previous_keys = keys;
    }
}

// Wait for any key to be pressed
void wait_keypress(void) {
    while (joypad() != 0) {
        wait_vbl_done();
    }
    while (joypad() == 0) {
        wait_vbl_done();
    }
    while (joypad() != 0) {
        wait_vbl_done();
    }
}

// A pre-calculated sigmoid-like curve.
// Maps inputs 0..31 (derived from 0..1024 cp) to output 0..64.
// This curve is steep at the start (small advantages look big) and flattens out.
const int8_t EVAL_CURVE[] = {
    0, 3, 8, 14, 20, 25, 30, 35,        // 0 - 250 cp:   Fast growth
    39, 43, 46, 49, 51, 53, 55, 56,     // 250 - 500 cp: Slowing down
    58, 59, 60, 61, 61, 62, 62, 63,     // 500 - 750 cp: Flattening
    63, 63, 64, 64, 64, 64, 64, 64      // 750+ cp:      Maxed out
};

// Scale eval fron int16_t in centipawns to +/-64 for plotting
int8_t scale_eval(int16_t cp) {
    int8_t is_negative = 0;

    // Safe Absolute Value Logic
    // We avoid doing math on INT16_MIN (-32768) directly to prevent overflow crashes.
    if (cp < 0) {
        if (cp == -32768)
            return -64;         // Hard clamp for the edge case
        cp = -cp;               // Safe to negate now
        is_negative = 1;
    }

    // Clamp Max Value
    // 0x0400 is 102If evaluation is > 10.24 pawns, just max out.
    if (cp >= 0x0400) {
        return is_negative ? -64 : 64;
    }

    // Calculate Index (Bit shift right by 5 is divide by 32)
    // We cast to uint8_t only AFTER shifting to ensure we have a valid small number.
    uint8_t index = (uint8_t) (cp >> 5);

    // Lookup
    int8_t val = EVAL_CURVE[index];

    // Return (No multiplication used)
    if (is_negative) {
        return -val;
    } else {
        return val;
    }
}

void update_eval_sprite(void) {
    int16_t eval = get_final_eval();
    int8_t scaled_eval = scale_eval(eval);
    /* int8_t scaled_eval = 0; */

    // Remember this flips if board is flipped!
    move_sprite(sprite_EVAL, BOARD_PIXEL_X - 8 - 1, BOARD_PIXEL_Y + 64 - 4 - scaled_eval);
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
    update_eval_sprite();
}

// Redraw the entire screen
void redraw(void) {
    // Clear borders
    print_xy(2, 0, " Game Boy Chess ");
    fill_bkg_rect(0, 0, 2, 18, tile_SPACE);
    fill_bkg_rect(18, 0, 2, 18, tile_SPACE);
    fill_bkg_rect(2, 17, 16, 1, tile_SPACE);

    draw_board();
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
    set_sprite_tile(sprite_EVAL, tile_CURSOR);
    set_sprite_tile(sprite_FROM_0, tile_MOVE_FROM_0);
    set_sprite_tile(sprite_FROM_1, tile_MOVE_FROM_0 + 1);
    set_sprite_tile(sprite_FROM_2, tile_MOVE_FROM_1);
    set_sprite_tile(sprite_FROM_3, tile_MOVE_FROM_1 + 1);
    set_sprite_tile(sprite_TO_0, tile_MOVE_TO_0);
    set_sprite_tile(sprite_TO_1, tile_MOVE_TO_0 + 1);
    set_sprite_tile(sprite_TO_2, tile_MOVE_TO_1);
    set_sprite_tile(sprite_TO_3, tile_MOVE_TO_1 + 1);
}

void update_cursor_sprite(void) {
    // Calculate Top-Left Pixel of the 16x16 Square
    uint8_t visual_row = 7 - cursor_y;

    uint8_t x = BOARD_PIXEL_X + (cursor_x << 4);
    uint8_t y = BOARD_PIXEL_Y + (visual_row << 4);

    // Position the 4 Sprites
    // Sprite 0: Top-Left
    move_sprite(sprite_CURSOR_0, x, y);

    // Sprite 1: Top-Right (x + 8)
    move_sprite(sprite_CURSOR_1, x + 8, y);

    // Sprite 2: Bottom-Left (y + 8)
    move_sprite(sprite_CURSOR_2, x, y + 8);

    // Sprite 3: Bottom-Right (x + 8, y + 8)
    move_sprite(sprite_CURSOR_3, x + 8, y + 8);
}

void hide_move_sprites(void) {
    for (uint8_t i = 0; i < 8; i++) {
        move_sprite(sprite_FROM_0 + i, 0, 0);
    }
}

void move_cursor(uint8_t sprite, uint8_t pos) {
    uint8_t x = pos & 0x7;
    uint8_t y = 7 - ((pos >> 4) & 0x7);
    x = BOARD_PIXEL_X + (x << 4);
    y = BOARD_PIXEL_Y + (y << 4);

    // Sprite 0: Top-Left
    move_sprite(sprite + 0, x, y);

    // Sprite 1: Top-Right (x + 8)
    move_sprite(sprite + 1, x + 8, y);

    // Sprite 2: Bottom-Left (y + 8)
    move_sprite(sprite + 2, x, y + 8);

    // Sprite 3: Bottom-Right (x + 8, y + 8)
    move_sprite(sprite + 3, x + 8, y + 8);
}

void update_move_sprites(Move *m) {
    move_cursor(sprite_FROM_0, m->from);
    move_cursor(sprite_TO_0, m->to);
}

// Helper: Convert cursor (0-7, 0-7) to 0x88 Index
uint8_t get_cursor_sq(void) {
    return (cursor_y * 16) + cursor_x;
}

void set_selected(uint8_t i) {
    selected_sq = i;
    set_cursor();
}

// Restart the game
void new_game(void) {
    hide_move_sprites();
    init_board();
    init_game_history();
    redraw();
    player_to_move = initial_player;
    initial_player = !initial_player;
    undo_valid = 0;
}

void in_game_menu(void) {
    draw_menu(in_game_menu_items, &in_game_menu_selection);
    redraw();
    switch (in_game_menu_selection) {
    case in_game_menu_UNDO:
        if (undo_valid >= 2) {
            unmake_move(&undo_move);
            undo_history();
            unmake_move(&undo_prev_move);
            undo_history();
            undo_valid = 0;
            hide_move_sprites();
            redraw();
        }
        break;
    case in_game_menu_SWAP:
        player_mapping ^= 3;
        break;
    case in_game_menu_NEW_GAME:
        new_game();
        break;
    case in_game_menu_LEVEL:
        draw_menu(level_menu_items, &level);
        redraw();
        break;
    case in_game_menu_PLAYERS:
        draw_menu(player_menu_items, &player_mapping);
        redraw();
        break;
    case in_game_menu_EXIT:
        break;
    }
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

    // Select button: show in game menu
    if (pressed & J_SELECT) {
        in_game_menu();
    }


    return move_to_make;
}

// Seed random number generator
void seed_rand(void) {
    uint16_t seed = DIV_REG;
    seed |= (uint16_t) LY_REG << 8;
    initrand(seed);
}

void main_menu(void) {
    while (1) {
        draw_menu(main_menu_items, &main_menu_selection);
        redraw();
        switch (main_menu_selection) {
        case 0:                // start game
            return;
        case 1:                // options
            draw_menu(level_menu_items, &level);
            redraw();
            break;
        case 2:                // player
            draw_menu(player_menu_items, &player_mapping);
            redraw();
            break;
        }
    }
}

void reset_spinner(void) {
    spinner_state = 0;
    busy_message = 0;
}

void draw_spinner(void) {
    set_bkg_tile_xy(0, 0, tile_SPINNER_0 + spinner_state);
    if (sys_time - message_time > 50) {
        if (busy_message == 0) {
            //               01234567890123456789
            print_xy(0, 17, "Thinking            ");
        } else {
            //               01234567890123456789
            print_xy(0, 17, "Hold B to stop      ");
        }
        busy_message ^= 1;
        message_time = sys_time;
    }
    spinner_state = (spinner_state + 1) & 3;
}

void clear_spinner(void) {
    set_bkg_tile_xy(0, 0, tile_SPACE);
    fill_bkg_rect(0, 17, 20, 1, tile_SPACE);
}

uint8_t my_game_callback(void) {
    draw_spinner();

    // If user presses B, interrupt the engine!
    uint8_t keys = joypad();

    if (keys & J_B) {
        play_error_sound();
        return 1;               // Signal engine to abort
    }

    return 0;                   // Continue thinking
}

// Run the game
int main(void) {
    // Load data into Background VRAM and Sprite VRAM
    // Not the most efficient but keeps things simple
    set_bkg_data(0, tiles_TILE_COUNT, tiles_tiles);
    set_sprite_data(0, tiles_TILE_COUNT, tiles_tiles);

    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;

    seed_rand();
    setup_sprites();
    new_game();
    update_cursor_sprite();
    init_sound();
    engine_callback = my_game_callback;

    main_menu();
    seed_rand();

    while (true) {
        Move *m;
        if ((player_mapping & (1 << player_to_move)) == 0) {
            m = handle_input();
        } else {
            reset_spinner();
            draw_spinner();
            next_move = computer_move(level);
            clear_spinner();
            m = &next_move;
        }
        if (m) {
            player_to_move = 1 - player_to_move;
            make_move(m);
            record_history();
            draw_board();

            // Record undo moves
            undo_prev_move = undo_move;
            undo_move = *m;
            undo_valid++;
            update_move_sprites(m);

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
                wait_keypress();
                new_game();
            }
        }
        wait_vbl_done();
    }
}
