#ifndef SPRITES_H
#define SPRITES_H

// Metasprite format: dx, dy, tile, attribute (palette, flip, priority)
// Terminated by 128 (0x80)

// Geeko McFly - Standing / Riding (24x24 px, 9 hardware sprites)
static const unsigned char GeekoRide[] = {
    0,   0, 0x00, 0,
    8,   0, 0x01, 0,
    16,  0, 0x02, 0,
    0,   8, 0x03, 0,
    8,   8, 0x04, 0,
    16,  8, 0x05, 0,
    0,  16, 0x06, 0,
    8,  16, 0x07, 0,
    16, 16, 0x08, 0,
    128
};

// Geeko McFly - Crouching / Ducking (24x16 px, 6 hardware sprites)
// Board must stay at the same height as when riding (dy = 17), so the art starts at dy = 7
static const unsigned char GeekoDuck[] = {
    0,   7, 0x09, 0,
    8,   7, 0x0A, 0,
    16,  7, 0x0B, 0,
    0,  15, 0x0C, 0,
    8,  15, 0x0D, 0,
    16, 15, 0x0E, 0,
    128
};

// Obstacle 1: Software Bug / Glitch (16x16 px, palette 2)
static const unsigned char SprBug[] = {
    0, 0, 0x10, 2,
    8, 0, 0x11, 2,
    0, 8, 0x12, 2,
    8, 8, 0x13, 2,
    128
};

// Obstacle 2: Tumbleweed (16x16 px, palette 1)
static const unsigned char SprTumbleweed[] = {
    0, 0, 0x14, 1,
    8, 0, 0x15, 1,
    0, 8, 0x16, 1,
    8, 8, 0x17, 1,
    128
};

// Obstacle 3: Airborne Flying Drone (16x16 px, palette 2)
static const unsigned char SprDrone[] = {
    0, 0, 0x18, 2,
    8, 0, 0x19, 2,
    0, 8, 0x1A, 2,
    8, 8, 0x1B, 2,
    128
};

// 88 MPH Fire Trail (8x8 px, palette 3)
static const unsigned char SprFireTrail[] = {
    0, 0, 0x20, 3,
    128
};

// Crash Particle Burst (8x8 px, palette 3)
static const unsigned char SprCrash[] = {
    0, 0, 0x21, 3,
    128
};

#endif // SPRITES_H
