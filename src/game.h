#ifndef GAME_H
#define GAME_H

// Constants and types shared by more than one module.

#define GROUND_Y           155    // Geeko hoverboard ground plane (screen Y of the sprite's top)
#define GEEKO_X            36     // Fixed horizontal screen position

// PPU control value the game runs with (neslib's bank_spr(1), bank_bg(0), NMI on); the HUD's scroll IRQ
// rewrites it with the nametable bit of the scroll.
#define PPU_CTRL_BASE      0x88

// Axis-aligned collision box. Layout matches what nesdoug's check_collision() reads:
// the first four bytes of both arguments must be x, y, width, height.
struct Box {
    unsigned char x;
    unsigned char y;
    unsigned char width;
    unsigned char height;
};

#endif // GAME_H
