#include <neslib.h>
#include <nesdoug.h>

#include "hud.h"
#include "progress.h"

#define SPLIT_TARGET_TILE  0x89   // background tile with one pixel on its last row
#define SPLIT_SPRITE_TILE  0x23   // sprite tile with one pixel on its first row

static void num_to_str5(unsigned int val, char *buf) {
    buf[0] = '0' + ((val / 10000) % 10);
    buf[1] = '0' + ((val / 1000) % 10);
    buf[2] = '0' + ((val / 100) % 10);
    buf[3] = '0' + ((val / 10) % 10);
    buf[4] = '0' + (val % 10);
}

static void num_to_str2(unsigned char val, char *buf) {
    buf[0] = '0' + ((val / 10) % 10);
    buf[1] = '0' + (val % 10);
}

void hud_draw_split_target(void) {
    vram_adr(NTADR_A(30, 2));
    vram_put(SPLIT_TARGET_TILE);
}

void hud_draw_static(void) {
    // Write stationary HUD to rows 1 and 2
    vram_adr(NTADR_A(1, 1));
    vram_write("HW26: HACK TO THE FUTURE", 24);

    vram_adr(NTADR_A(1, 2));
    vram_write("SPD:20MPH  SCR:00000 HI:00000", 29);

    hud_draw_split_target();
}

void hud_sprite_zero(void) {
    // Sprite 0 pixel row 0 is drawn on scanline Y + 1 = 23, the last HUD scanline, and
    // overlaps the target tile's pixel at NTADR_A(30, 2). split() then scrolls from scanline
    // 24, the first playfield line. Attr 0x20 = behind background (hidden by that pixel).
    // (A hit on scanline 21 scrolled the bottom of the HUD text: glyph row 6 is scanline 22.)
    oam_spr(240, 22, SPLIT_SPRITE_TILE, 0x20);
}

void hud_update(void) {
    static char buf_spd[2];
    static char buf_scr[5];
    static char buf_hi[5];

    num_to_str2(mph, buf_spd);
    multi_vram_buffer_horz(buf_spd, 2, NTADR_A(5, 2));

    num_to_str5(score, buf_scr);
    multi_vram_buffer_horz(buf_scr, 5, NTADR_A(16, 2));

    num_to_str5(hi_score, buf_hi);
    multi_vram_buffer_horz(buf_hi, 5, NTADR_A(25, 2));
}

void hud_scroll(unsigned char scrolling) {
    if (scrolling) {
        split(scroll_x);
    }
    set_scroll_x(0);
    set_scroll_y(0);
}
