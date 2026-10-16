#ifndef SPRITES_H
#define SPRITES_H

// Metasprite format: dx, dy, tile, attribute (palette, flip, priority)
// Terminated by 128 (0x80)

// Geeko McFly - Standing / Riding (16x16 px, 4 hardware sprites)
static const unsigned char GeekoRide[] = {
    0,  0, 0x00, 0,
    8,  0, 0x01, 0,
    0,  8, 0x02, 0,
    8,  8, 0x03, 0,
    128
};

// Geeko McFly - Crouching / Ducking (16x16 px, 4 hardware sprites)
// Same 16x16 cell as riding: the board is in the same rows, only the body is squashed
static const unsigned char GeekoDuck[] = {
    0,  0, 0x04, 0,
    8,  0, 0x05, 0,
    0,  8, 0x06, 0,
    8,  8, 0x07, 0,
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
