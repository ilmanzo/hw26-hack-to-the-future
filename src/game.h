#ifndef GAME_H
#define GAME_H

// Constants and types shared by more than one module.

#define GROUND_Y           148    // Geeko hoverboard ground plane (screen Y of the sprite's top)
#define GEEKO_X            36     // Fixed horizontal screen position

// Axis-aligned collision box. Layout matches what nesdoug's check_collision() reads:
// the first four bytes of both arguments must be x, y, width, height.
struct Box {
    unsigned char x;
    unsigned char y;
    unsigned char width;
    unsigned char height;
};

#endif // GAME_H
