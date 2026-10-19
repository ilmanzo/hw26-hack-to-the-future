#ifndef HUD_H
#define HUD_H

// Stationary HUD (nametable rows 1-2) above the scrolling playfield.
//
// The split is an MMC3 scanline IRQ: after the 24 HUD scanlines it writes the scroll registers, so
// everything below the HUD scrolls and the main loop never waits for it. tests/smoke.lua asserts
// the scroll lands on scanline 23.

// Static text. Rendering must be off. in_room: the second row shows the
// terminals hacked instead of the speed.
void hud_draw_static(unsigned char in_room);

// 5 digits of v into buf (no terminator).
void hud_num5(unsigned int v, char *buf);

// Refresh clock, multiplier, speed (or terminals), score and hi-score through the VRAM buffer (safe
// while rendering). hacked / total: terminals, only used in a room.
void hud_update(unsigned char in_room, unsigned char hacked, unsigned char total);

// Point the MMC3 IRQ at the HUD's array. Call once at boot, after the mapper is set up.
void hud_irq_init(void);

// Start-of-frame scroll registers and the IRQ for this frame. scrolling = scroll_x below the HUD;
// otherwise the whole screen stays at 0.
void hud_scroll(unsigned char scrolling);

// Show a year (4 digits) in the HUD's year field, through the VRAM buffer (the warp rolls it).
void hud_year_show(unsigned int year);

#endif // HUD_H
