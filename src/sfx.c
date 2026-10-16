#include <neslib.h>

#include "sfx.h"

// Sound effect ids: the song order in assets/audio/sfx.txt.
#define SFX_JUMP    0
#define SFX_DUCK    1
#define SFX_BEEP    2
#define SFX_WARP    3
#define SFX_CRASH   4

// FamiTone2 effect slots (0..3): one effect at a time per slot, so effects on the same APU
// channel share a slot and the newer one replaces the older.
#define SLOT_PULSE1   0
#define SLOT_PULSE2   1
#define SLOT_TRIANGLE 2
#define SLOT_NOISE    3

extern const unsigned char music_data[];
extern const unsigned char sounds_data[];

void sfx_init(void) {
    music_init(music_data);
    sounds_init(sounds_data);
}

void sfx_music_start(void) {
    music_play(0);
}

void sfx_music_stop(void) {
    music_stop();
}

void sfx_jump(void)  { sfx_play(SFX_JUMP,  SLOT_PULSE1); }
void sfx_duck(void)  { sfx_play(SFX_DUCK,  SLOT_PULSE1); }
void sfx_beep(void)  { sfx_play(SFX_BEEP,  SLOT_PULSE2); }
void sfx_warp(void)  { sfx_play(SFX_WARP,  SLOT_TRIANGLE); }
void sfx_crash(void) { sfx_play(SFX_CRASH, SLOT_NOISE); }
