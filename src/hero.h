#ifndef HERO_H
#define HERO_H

#include "game.h"

// Geeko in a room: run with inertia, jump, collide with the room's solid tiles. State is private to
// hero.c. Numbers: plan.md 4a. (player.c is Geeko in the race.)

// Stand on the surface row (tile row) at tile column col, facing right.
void hero_reset(unsigned char row, unsigned char col);

// One frame. pad = buttons held, pad_new = pressed this frame. hacking: Geeko is hacking a
// terminal (D-pad ignored, hack pose).
void hero_update(unsigned char pad, unsigned char pad_new, unsigned char hacking);

// A foe touched Geeko: knock back, away from foe_x (pixel x of the foe's centre).
void hero_knock(unsigned char foe_x);

void hero_get_box(struct Box *box);
unsigned char hero_x(void);        // sprite left edge, pixels
unsigned char hero_y(void);        // sprite top edge, pixels
unsigned char hero_grounded(void);
unsigned char hero_still(void);    // horizontal speed is 0

// hacking: draw the hack pose; blinks while invulnerable.
void hero_draw(unsigned char hacking);

#endif // HERO_H
