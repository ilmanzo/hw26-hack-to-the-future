#include <neslib.h>
#include <nesdoug.h>

#include "gfx.h"
#include "hud.h"
#include "map.h"
#include "palettes.h"
#include "sprites.h"

// Background tiles (pattern table 0, see tools/build_chr.py)
#define TILE_PATH        0xa4
#define TILE_NODE_WAIT   0xa8    // 2x2 each
#define TILE_NODE_DONE   0xac
#define TILE_NODE_BOSS   0xb0

#define NODES            5
#define NODE_ROW         10      // top row of the 2x2 nodes (even, for the palette)

static const unsigned char node_col[NODES] = { 4, 10, 16, 22, 28 };   // left column (even)
static const char years[NODES][4] = { {'1','9','9','0'}, {'2','0','0','0'}, {'2','0','1','0'},
                                      {'2','0','2','0'}, {'2','0','5','4'} };
static const char *const names[NODES] = { "UNIX LABS", "DOT-COM FARM", "CLOUD CLEANROOM", "AI EDGE",
                                          "THE MAINFRAME" };
static const unsigned char name_len[NODES] = { 9, 12, 15, 7, 13 };

void map_show(unsigned char epoch) {
    unsigned char i;

    gfx_screen_off();
    pal_bg(room_bg_palette);

    gfx_wipe(3);                         // the HUD rows stay

    vram_adr(NTADR_A(12, 5));
    vram_write("TIME MAP", 8);

    for (i = 0; i < NODES; ++i) {
        unsigned char col = node_col[i];
        if (i + 1 < NODES) {             // the path to the next node
            vram_adr(NTADR_A(col + 2, NODE_ROW + 1));
            vram_fill(TILE_PATH, node_col[i + 1] - col - 2);
        }
        gfx_block(col, NODE_ROW, i == NODES - 1 ? TILE_NODE_BOSS : i < epoch ? TILE_NODE_DONE : TILE_NODE_WAIT, 2, 2);
        gfx_attr_set(col, NODE_ROW, i == NODES - 1 ? 2 : 1);
        vram_adr(NTADR_A(col - 1, NODE_ROW + 3));
        vram_write(years[i], 4);
    }

    vram_adr(NTADR_A(11, 17));
    vram_write("NEXT STOP", 9);
    vram_adr(NTADR_A((32 - name_len[epoch]) / 2, 19));
    vram_write(names[epoch], name_len[epoch]);
    vram_adr(NTADR_A(10, 24));
    vram_write("PRESS A TO GO", 13);

    gfx_attr_flush();

    hud_draw_static(0);
    ppu_on_all();
}

void map_draw_geeko(unsigned char epoch, unsigned char timer) {
    if ((timer >> 3) & 1) return;        // blink: 8 frames on, 8 off
    oam_meta_spr(node_col[epoch] * 8, NODE_ROW * 8 - 20, GeekoRide);
}
