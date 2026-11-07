/* Shows one screen drawn by castle_art.py: palettes, tiles in both VRAM
 * banks, attributes and the cursor sprites. A preview loader, not the game. */
#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "screen.h"
#include "sprites.h"

static const palette_color_t spr_pal[] = {RGB8(0, 0, 0), RGB8(255, 255, 255), RGB8(220, 150, 30), RGB8(16, 16, 20)};

static void needs_color(void) {
    /* The mockup's DMG screen lives in the first prototype; here just stop. */
    BGP_REG = 0xE4;
    DISPLAY_ON;
    while (1) wait_vbl_done();
}

void main(void) {
    uint8_t i, n, k, x, y;
    DISPLAY_OFF;
    if (_cpu != CGB_TYPE) needs_color();
    set_bkg_palette(0, 8, screen_pal);
    set_sprite_palette(0, 1, spr_pal);
    n = screen_tile_count > 256u ? 0u : (uint8_t)screen_tile_count;
    VBK_REG = 0;
    if (screen_tile_count >= 256u) set_bkg_data(0, 0, screen_tiles); /* 0 count = 256 tiles */
    else set_bkg_data(0, n, screen_tiles);
    if (screen_tile_count > 256u) {
        VBK_REG = 1;
        set_bkg_data(0, (uint8_t)(screen_tile_count - 256u), screen_tiles + 256u * 16u);
    }
    VBK_REG = 1;
    set_bkg_tiles(0, 0, 20, 18, screen_attr);
    VBK_REG = 0;
    set_bkg_tiles(0, 0, 20, 18, screen_map);
    set_sprite_data(0, SPRITE_TILES, sprite_tiles);
    k = 0;
    for (i = 0; i < screen_sprite_count; ++i) {
        x = screen_sprites[i * 3u + 1u];
        y = screen_sprites[i * 3u + 2u];
        if (screen_sprites[i * 3u] == 0u) {
            set_sprite_tile(k, 0); move_sprite(k++, (uint8_t)(x + 8u), (uint8_t)(y + 16u));
            set_sprite_tile(k, 1); move_sprite(k++, (uint8_t)(x + 16u), (uint8_t)(y + 16u));
            set_sprite_tile(k, 2); move_sprite(k++, (uint8_t)(x + 8u), (uint8_t)(y + 24u));
            set_sprite_tile(k, 3); move_sprite(k++, (uint8_t)(x + 16u), (uint8_t)(y + 24u));
        } else {
            set_sprite_tile(k, 4); move_sprite(k++, (uint8_t)(x + 12u), (uint8_t)(y + 20u));
        }
    }
    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;
    while (1) wait_vbl_done();
}
