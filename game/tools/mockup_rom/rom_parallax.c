/* Shows the title drawn by castle_art.py with drifting clouds. A preview
 * loader, not the game.
 * Rows 0-4 (the wordmark) are shown through the window layer, which never
 * scrolls. Rows 5 and 6 are two cloud bands; a LYC interrupt gives each its
 * own horizontal scroll, and the castle rows below get scroll zero. */
#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "screen.h"
#include "sprites.h"

static uint8_t far_x, near_x, sub;

static void lcd_isr(void) {
    switch (LYC_REG) {
    case 39u: LCDC_REG &= ~0x20u; SCX_REG = far_x; LYC_REG = 47u; break;  /* window off below the wordmark */
    case 47u: SCX_REG = near_x; LYC_REG = 55u; break;
    default:  SCX_REG = 0u; LYC_REG = 39u; break;
    }
}

static void needs_color(void) {
    BGP_REG = 0xE4;
    DISPLAY_ON;
    while (1) wait_vbl_done();
}

void main(void) {
    uint8_t x, y, t, a;
    DISPLAY_OFF;
    if (_cpu != CGB_TYPE) needs_color();
    set_bkg_palette(0, 8, screen_pal);
    VBK_REG = 0;
    if (screen_tile_count >= 256u) set_bkg_data(0, 0, screen_tiles);
    else set_bkg_data(0, (uint8_t)screen_tile_count, screen_tiles);
    if (screen_tile_count > 256u) {
        VBK_REG = 1;
        set_bkg_data(0, (uint8_t)(screen_tile_count - 256u), screen_tiles + 256u * 16u);
    }
    VBK_REG = 1;
    set_bkg_tiles(0, 0, 20, 18, screen_attr);
    set_win_tiles(0, 0, 20, 5, screen_attr);
    VBK_REG = 0;
    set_bkg_tiles(0, 0, 20, 18, screen_map);
    set_win_tiles(0, 0, 20, 5, screen_map);
    /* the cloud rows wrap across the 32-column map: fill the off-screen
     * columns with the plain sky tile at the row's left edge */
    for (y = 5u; y < 7u; ++y) {
        t = screen_map[y * 20u];
        a = screen_attr[y * 20u];
        for (x = 20u; x < 32u; ++x) {
            VBK_REG = 1; set_bkg_tile_xy(x, y, a);
            VBK_REG = 0; set_bkg_tile_xy(x, y, t);
        }
    }
    move_win(7, 0);
    LCDC_REG |= 0x40u;  /* window map at 9C00 */
    STAT_REG = 0x40u;
    LYC_REG = 39u;
    add_LCD(lcd_isr);
    set_interrupts(VBL_IFLAG | LCD_IFLAG);
    SHOW_BKG;
    SHOW_WIN;
    DISPLAY_ON;
    far_x = near_x = sub = 0u;
    while (1) {
        wait_vbl_done();
        LCDC_REG |= 0x20u;  /* window back on for the next frame's top rows */
        ++sub;
        if ((sub & 3u) == 0u) ++far_x;   /* far clouds: a pixel every 4 frames */
        if ((sub & 1u) == 0u) ++near_x;  /* near clouds: a pixel every 2 frames */
    }
}
