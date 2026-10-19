#include <neslib.h>
#include <nesdoug.h>

#include "gfx.h"
#include "hud.h"
#include "progress.h"
#include "screens.h"
#include "tuning.h"

// Background tiles (pattern table 0, see tools/build_chr.py)
#define TILE_EMPTY         0x00
#define TILE_HORIZON       0x80
#define TILE_ROAD          0x81
#define TILE_ROAD_DASH     0x82
#define TILE_BEDROCK       0x83
#define TILE_MOUNTAIN_L    0x84
#define TILE_MOUNTAIN_R    0x85
#define TILE_CLOUD_L       0x86
#define TILE_CLOUD_R       0x87

void background_draw(unsigned int base_addr) {
    unsigned char col, row;

    // Rows 0..2: Reserved for HUD
    // Row 3..14: Cyber Sky & Neon Mountains
    for (row = 3; row < 15; ++row) {
        vram_adr(base_addr + (row * 32));
        for (col = 0; col < 32; ++col) {
            if (row == 5 && (col == 6 || col == 22)) {
                vram_put(TILE_CLOUD_L);
            } else if (row == 5 && (col == 7 || col == 23)) {
                vram_put(TILE_CLOUD_R);
            } else if (row == 12 && (col % 8 == 0)) {
                vram_put(TILE_MOUNTAIN_L); // Mountain peak
            } else if (row == 12 && (col % 8 == 1)) {
                vram_put(TILE_MOUNTAIN_R);
            } else if (row == 13 && (col % 8 == 7 || col % 8 == 0 || col % 8 == 1 || col % 8 == 2)) {
                vram_put(TILE_BEDROCK); // Mountain body
            } else if (row == 14 && (col % 8 >= 6 || col % 8 <= 3)) {
                vram_put(TILE_BEDROCK); // Mountain base
            } else {
                vram_put(TILE_EMPTY); // Empty sky
            }
        }
    }

    // Rows 15..20: Cyber Horizon
    for (row = 15; row < 21; ++row) {
        vram_adr(base_addr + (row * 32));
        for (col = 0; col < 32; ++col) {
            vram_put(TILE_EMPTY);
        }
    }

    // Row 21: Road Horizon line
    vram_adr(base_addr + (21 * 32));
    for (col = 0; col < 32; ++col) {
        vram_put(TILE_HORIZON);
    }

    // Rows 22..24: Road surface with traces & center dash
    for (row = 22; row < 25; ++row) {
        vram_adr(base_addr + (row * 32));
        for (col = 0; col < 32; ++col) {
            if (row == 23 && (col % 4 < 2)) {
                vram_put(TILE_ROAD_DASH); // Dashed center highway line
            } else {
                vram_put(TILE_ROAD); // Circuit trace road
            }
        }
    }

    // Rows 25..29: Solid Cyber Bedrock
    for (row = 25; row < 30; ++row) {
        vram_adr(base_addr + (row * 32));
        for (col = 0; col < 32; ++col) {
            vram_put(TILE_BEDROCK);
        }
    }
}

void title_screen_draw(void) {
    ppu_off();

    // Clear Nametable 0
    vram_adr(NTADR_A(0, 0));
    vram_fill(TILE_EMPTY, 1024);

    background_draw(NTADR_A(0, 0));

    // Title Banners (font tiles are indexed by ASCII; see tools/build_chr.py for the glyphs)
    vram_adr(NTADR_A(7, 4));
    vram_write("HACK TO THE FUTURE", 18);

    vram_adr(NTADR_A(9, 6));
    vram_write("GEEKO MCFLY '85", 15);

    vram_adr(NTADR_A(4, 8));
    vram_write("OPENSUSE HACK WEEK 26", 21);

    vram_adr(NTADR_A(2, 11));
    vram_write("\"WHERE WE GO, NO ROADS!\"", 24);

    vram_adr(NTADR_A(8, 16));
    vram_write("PRESS START OR A", 16);

    vram_adr(NTADR_A(6, 18));
    vram_write("A: JUMP   DOWN: DUCK", 20);

#if DEV_SELECT_ROOM
    vram_adr(NTADR_A(6, 20));
    vram_write("SELECT: ROOM TEST (DEV)", 23);
#endif

    ppu_on_all();
}

void game_over_show(unsigned char finished) {
    static char buf[5];

    // Rendering off: this is more text than the VRAM buffer can flush in one vblank.
    ppu_off();
    if (finished) gfx_wipe(3);           // came from the map: wipe it (the HUD stays)
    if (finished) {
        vram_adr(NTADR_A(8, 6));
        vram_write("TO BE CONTINUED", 15);
    } else {
        vram_adr(NTADR_A(11, 6));
        vram_write("GAME OVER", 9);
    }
    vram_adr(NTADR_A(10, 8));
    vram_write("SCORE ", 6);
    hud_num5(score, buf);
    vram_write(buf, 5);
    vram_adr(NTADR_A(7, 9));
    vram_write("RANK ", 5);
    vram_write(rank_name(), 13);
    vram_adr(NTADR_A(7, 11));
    vram_write("PRESS A TO RESTART", 18);
    ppu_on_all();
}

void result_card_draw(unsigned char clean, unsigned char hacked, unsigned char total) {
    static char buf[5];
    unsigned int secs = race_frames / 60;

    ppu_off();
    gfx_wipe(3);                         // the HUD rows stay

    vram_adr(NTADR_A(8, 6));
    vram_write("INSTALL COMPLETE", 16);

    vram_adr(NTADR_A(8, 8));
    vram_write("RACE TIME ", 10);
    buf[0] = '0' + secs / 60;
    buf[1] = ':';
    buf[2] = '0' + (secs % 60) / 10;
    buf[3] = '0' + secs % 10;
    vram_write(buf, 4);

    vram_adr(NTADR_A(8, 9));
    vram_write("TERMINALS ", 10);
    buf[0] = '0' + hacked;
    buf[1] = '/';
    buf[2] = '0' + total;
    vram_write(buf, 3);

    vram_adr(NTADR_A(8, 10));
    if (clean) {
        vram_write("CLEAN ROOM BONUS", 16);
    } else {
        vram_write("HIT: NO BONUS", 13);
    }

    vram_adr(NTADR_A(8, 12));
    vram_write("SCORE ", 6);
    hud_num5(score, buf);
    vram_write(buf, 5);

    vram_adr(NTADR_A(8, 13));
    vram_write("RANK  ", 6);
    vram_write(rank_name(), 13);

    vram_adr(NTADR_A(6, 16));
    vram_write("PRESS A TO CONTINUE", 19);

    ppu_on_all();
}
