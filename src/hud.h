#ifndef HUD_H
#define HUD_H

// Stationary HUD (nametable rows 1-2) above the scrolling playfield.
//
// The split works with sprite 0: its pixel overlaps a one-pixel background tile at the HUD's
// last scanline, nesdoug's split() waits for that hit and then applies the scroll, so
// everything below the HUD scrolls. Sprite, target tile and scroll write must stay in step;
// tests/smoke.lua asserts the scroll lands on scanline 23.

// Static text + the sprite-0 target tile. Rendering must be off. in_room: the second row shows the
// terminals hacked instead of the speed.
void hud_draw_static(unsigned char in_room);

// 5 digits of v into buf (no terminator).
void hud_num5(unsigned int v, char *buf);

// Only the target tile (the title screen has no HUD text but still needs it).
void hud_draw_split_target(void);

// Put sprite 0 in OAM slot 0. Call first, after oam_clear().
void hud_sprite_zero(void);

// Refresh clock, multiplier, speed (or terminals), score and hi-score through the VRAM buffer (safe
// while rendering). hacked / total: terminals, only used in a room.
void hud_update(unsigned char in_room, unsigned char hacked, unsigned char total);

// Start-of-frame scroll registers. scrolling = apply scroll_x below the HUD (blocks until
// the sprite-0 hit); otherwise the whole screen stays at 0.
void hud_scroll(unsigned char scrolling);

#endif // HUD_H
