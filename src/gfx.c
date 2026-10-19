#include <neslib.h>
#include <nesdoug.h>

#include "gfx.h"

static unsigned char attr[64];    // the attribute table of nametable A: one byte per 4x4 tiles

void gfx_wipe(unsigned char first_row) {
    unsigned char i;

    vram_adr(NTADR_A(0, first_row));
    vram_fill(0, (30 - first_row) * 32);
    for (i = 0; i < 64; ++i) attr[i] = 0;
    gfx_attr_flush();
}

void gfx_block(unsigned char col, unsigned char row, unsigned char first, unsigned char w, unsigned char h) {
    unsigned char x, y;

    for (y = 0; y < h; ++y) {
        vram_adr(NTADR_A(col, row + y));
        for (x = 0; x < w; ++x) vram_put(first++);
    }
}

void gfx_attr_set(unsigned char col, unsigned char row, unsigned char pal) {
    unsigned char i = (row >> 2) * 8 + (col >> 2);
    unsigned char shift = ((row >> 1) & 1) * 4 + ((col >> 1) & 1) * 2;

    attr[i] = (attr[i] & ~(3 << shift)) | (pal << shift);
}

void gfx_attr_flush(void) {
    vram_adr(0x23c0);
    vram_write(attr, 64);
}
