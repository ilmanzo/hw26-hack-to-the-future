#ifndef SFX_H
#define SFX_H

// Sound effects: raw APU register writes, no sound engine.
// Channel use: Pulse 1 = jump/duck, Pulse 2 = beep, Triangle = warp, Noise = crash.

void sfx_init(void);
void sfx_jump(void);
void sfx_duck(void);
void sfx_beep(void);
void sfx_warp(void);
void sfx_crash(void);

#endif // SFX_H
