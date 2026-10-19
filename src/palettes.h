#ifndef PALETTES_H
#define PALETTES_H

// NES Color Palette Constants
#define C_BLACK        0x0f
#define C_SKY_BLUE     0x11   // the race's sky: the background colour of every race palette
#define C_DARK_GRAY    0x00
#define C_MED_GRAY     0x10
#define C_WHITE        0x30

#define C_CYBER_BLUE   0x02
#define C_NEON_CYAN    0x2c
#define C_ELECTRIC_BLU 0x1c
#define C_PURPLE       0x12

#define C_GEEKO_GREEN  0x29
#define C_DARK_GREEN   0x19
#define C_MCFLY_ORANGE 0x27
#define C_FIRE_YELLOW  0x28
#define C_FIRE_RED     0x16
#define C_DIRT_BROWN   0x17

// Background Palette (4 sub-palettes of 4 colors each)
static const unsigned char bg_palette[16] = {
    C_SKY_BLUE, C_DARK_GRAY, C_MED_GRAY, C_WHITE,        // 0: Sky / Text
    C_SKY_BLUE, C_CYBER_BLUE, C_NEON_CYAN, C_GEEKO_GREEN, // 1: Cyber Road / Grid
    C_SKY_BLUE, C_PURPLE, C_MCFLY_ORANGE, C_WHITE,       // 2: Mountains & Sun
    C_SKY_BLUE, C_DARK_GRAY, C_ELECTRIC_BLU, C_WHITE     // 3: HUD / Accents
};

// Room background palette (room 1, 1990): solid brick and bar, terminals, gate, text
static const unsigned char room_bg_palette[16] = {
    C_BLACK, 0x06, C_FIRE_RED, C_WHITE,               // 0: brick (mortar, brick, highlight), hack bar, text
    C_BLACK, C_CYBER_BLUE, C_PURPLE, 0x2a,            // 1: terminals (dark blue, blue, green)
    C_BLACK, C_FIRE_RED, C_MCFLY_ORANGE, 0x2a,        // 2: gate (red, orange, green)
    C_BLACK, C_DARK_GRAY, C_ELECTRIC_BLU, C_WHITE     // 3: spare
};

// Sprite Palette (4 sub-palettes of 4 colors each)
static const unsigned char spr_palette[16] = {
    C_BLACK, C_GEEKO_GREEN, C_DARK_GREEN, C_WHITE,    // 0: Geeko McFly (Light green, Dark green, White)
    C_BLACK, C_DIRT_BROWN, C_MCFLY_ORANGE, C_FIRE_YELLOW, // 1: Tumbleweed / Hazard
    C_BLACK, C_ELECTRIC_BLU, C_PURPLE, C_WHITE,       // 2: Bugs / Cyber Drone
    C_BLACK, C_FIRE_RED, C_MCFLY_ORANGE, C_FIRE_YELLOW // 3: 88 MPH Fire Trail & FX
};

#endif // PALETTES_H
