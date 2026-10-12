#ifndef SCREENS_H
#define SCREENS_H

// Full-screen layouts: the scrolling background and the title / game-over text.

// Draw sky, mountains, road and bedrock into the nametable at base_addr (NTADR_A / NTADR_B).
// Rows 0-2 are left alone for the HUD. Rendering must be off.
void background_draw(unsigned int base_addr);

// Turns rendering off, draws the title screen, turns rendering back on.
void title_screen_draw(void);

// Queue the CRASH banner through the VRAM buffer (safe while rendering).
void crash_banner_show(void);

#endif // SCREENS_H
