#include <peekpoke.h>

#include "sfx.h"

void sfx_init(void) {
    POKE(0x4015, 0x0f); // Enable Pulse 1, 2, Triangle, Noise
}

void sfx_jump(void) {
    POKE(0x4000, 0x98); // Pulse 1: 50% duty, decay
    POKE(0x4001, 0x82); // Sweep up
    POKE(0x4002, 0x70); // Base pitch
    POKE(0x4003, 0x98); // Trigger length
}

void sfx_duck(void) {
    POKE(0x4000, 0x58); // Pulse 1: 25% duty, short chirp
    POKE(0x4001, 0x80);
    POKE(0x4002, 0xa0);
    POKE(0x4003, 0x98);
}

void sfx_beep(void) {
    POKE(0x4004, 0x8c); // Pulse 2: 50% duty
    POKE(0x4005, 0x7f);
    POKE(0x4006, 0x22); // High chime (C6)
    POKE(0x4007, 0x98);
}

void sfx_warp(void) {
    POKE(0x4008, 0x7f); // Triangle tone
    POKE(0x400a, 0x18);
    POKE(0x400b, 0x98);
}

void sfx_crash(void) {
    POKE(0x400c, 0x3f); // Noise channel: max volume
    POKE(0x400e, 0x0e); // Low rumbling white noise
    POKE(0x400f, 0x98); // Trigger
}
