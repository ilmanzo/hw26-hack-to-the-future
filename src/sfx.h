#ifndef SFX_H
#define SFX_H

// Music and sound effects, both through FamiTone2 (neslib). Data lives in assets/audio/
// (regenerate with tools/export_audio.sh). Do not poke the APU registers directly: the engine
// rewrites every channel each frame.

void sfx_init(void);   // load the music and effect data; call once at boot

void sfx_music_start(void);   // song 0 from the beginning
void sfx_music_stop(void);

void sfx_jump(void);
void sfx_duck(void);
void sfx_beep(void);
void sfx_warp(void);
void sfx_crash(void);

#endif // SFX_H
