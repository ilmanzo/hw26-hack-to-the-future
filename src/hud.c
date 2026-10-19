#include <neslib.h>
#include <nesdoug.h>

#include "hud.h"
#include "progress.h"

#define SPLIT_TARGET_TILE  0x89   // background tile with one pixel on its last row
#define SPLIT_SPRITE_TILE  0x23   // sprite tile with one pixel on its first row

// What the screen shows now, so hud_update() only writes changes (0xff: unknown, write it).
static unsigned int last_secs, last_score, last_hi;
static unsigned char last_mult, last_a, last_total;

void hud_num5(unsigned int val, char *buf) {
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

void hud_draw_static(unsigned char in_room) {
    const char *year = year_text();

    // Stationary HUD in nametable rows 1 and 2. Digit columns used by hud_update():
    // clock 6-10, year 18-21, multiplier 25; speed 5-6 or hacked 6 / total 8, score 16-20, hi 25-29.
    vram_adr(NTADR_A(1, 1));
    vram_write("TIME 10:00  YEAR ", 17);
    vram_write(year, 4);
    vram_write("  X1", 4);

    vram_adr(NTADR_A(1, 2));
    if (in_room) {
        vram_write("HACK 0/0   SCR:00000 HI:00000", 29);
    } else {
        vram_write("SPD:20MPH  SCR:00000 HI:00000", 29);
    }

    hud_draw_split_target();

    last_secs = last_score = last_hi = 0xffff;
    last_mult = last_a = last_total = 0xff;
}

void hud_sprite_zero(void) {
    // Sprite 0 pixel row 0 is drawn on scanline Y + 1 = 23, the last HUD scanline, and
    // overlaps the target tile's pixel at NTADR_A(30, 2). split() then scrolls from scanline
    // 24, the first playfield line. Attr 0x20 = behind background (hidden by that pixel).
    // (A hit on scanline 21 scrolled the bottom of the HUD text: glyph row 6 is scanline 22.)
    oam_spr(240, 22, SPLIT_SPRITE_TILE, 0x20);
}

void hud_update(unsigned char in_room, unsigned char hacked, unsigned char total) {
    static char buf_time[5];
    static char buf_one[1];
    static char buf_two[2];
    static char buf_num[5];
    unsigned int secs = (time_left + 59) / 60;   // rounded up: 10:00 until a full second has passed
    unsigned char a = in_room ? hacked : mph;

    // Only what changed goes into the VRAM buffer: the NMI can flush about 100 bytes per vblank.
    if (secs != last_secs) {
        unsigned char m = secs / 60;
        unsigned char s = secs % 60;
        last_secs = secs;
        buf_time[0] = '0' + m / 10;
        buf_time[1] = '0' + m % 10;
        buf_time[2] = ':';
        buf_time[3] = '0' + s / 10;
        buf_time[4] = '0' + s % 10;
        multi_vram_buffer_horz(buf_time, 5, NTADR_A(6, 1));
    }

    if (mult != last_mult) {
        last_mult = mult;
        buf_one[0] = '0' + mult;
        multi_vram_buffer_horz(buf_one, 1, NTADR_A(25, 1));
    }

    if (a != last_a || total != last_total) {
        last_a = a;
        last_total = total;
        if (in_room) {
            buf_two[0] = '0' + hacked;
            buf_two[1] = '/';
            multi_vram_buffer_horz(buf_two, 2, NTADR_A(6, 2));
            buf_one[0] = '0' + total;
            multi_vram_buffer_horz(buf_one, 1, NTADR_A(8, 2));
        } else {
            num_to_str2(mph, buf_two);
            multi_vram_buffer_horz(buf_two, 2, NTADR_A(5, 2));
        }
    }

    if (score != last_score) {
        last_score = score;
        hud_num5(score, buf_num);
        multi_vram_buffer_horz(buf_num, 5, NTADR_A(16, 2));
    }

    if (hi_score != last_hi) {
        last_hi = hi_score;
        hud_num5(hi_score, buf_num);
        multi_vram_buffer_horz(buf_num, 5, NTADR_A(25, 2));
    }
}

void hud_scroll(unsigned char scrolling) {
    if (scrolling) {
        split(scroll_x);
    }
    set_scroll_x(0);
    set_scroll_y(0);
}
