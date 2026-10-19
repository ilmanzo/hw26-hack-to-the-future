#ifndef GFX_H
#define GFX_H

// PPU drawing helpers shared by the room, the time map and the cards. Rendering must be off.

// Blank the nametable from tile row first_row down (rows below 3 leave the HUD alone) and the whole
// attribute table: palette 0 everywhere. Starts a fresh attribute buffer.
void gfx_wipe(unsigned char first_row);

// A w x h block of tiles first, first+1, ... (row-major) with its top-left at (col, row).
void gfx_block(unsigned char col, unsigned char row, unsigned char first, unsigned char w, unsigned char h);

// Give the 2x2 tile area containing (col, row) palette pal. Buffered until gfx_attr_flush().
void gfx_attr_set(unsigned char col, unsigned char row, unsigned char pal);

// Write the buffered attribute table to the PPU.
void gfx_attr_flush(void);

#endif // GFX_H
