#ifndef GEEKO_H
#define GEEKO_H

#include "game.h"

// Geeko's body, shared by every level: the race (player.c) and the rooms and boss (hero.c) use the
// same jump, fall and run code, so Geeko feels the same everywhere. What differs is only the
// ground: the race has a flat road, rooms collide with tiles (hero.c). Numbers: plan.md 4a.

struct Geeko {
    unsigned int x, y;          // 8.8 fixed point, sprite top-left
    int vx, vy;                 // 8.8 fixed point per frame
    unsigned char grounded;
    unsigned char facing_left;
};

// 8.8 fixed point per frame
#define GEEKO_GRAVITY    0x38
#define GEEKO_JUMP       0x4c0
#define GEEKO_JUMP_CUT   0x180   // upward speed after letting go of the jump button while rising
#define GEEKO_FALL_MAX   0x700   // under one tile per frame: no tunnelling through a 1-tile platform
#define GEEKO_ACCEL      0x18    // run acceleration on the ground
#define GEEKO_AIR        0x0c    // steering in the air
#define GEEKO_RUN_MAX    0x200   // 2 px per frame
#define GEEKO_FRICTION   0x10    // glide to a stop
#define GEEKO_SKID       0x30    // turning round on the board

// Hitbox inside the 16x16 sprite: x+2, width 12; standing y+1, height 15; crouched y+6, height 10.
#define GEEKO_BOX_X      2
#define GEEKO_BOX_W      12
#define GEEKO_BOX_Y      1
#define GEEKO_BOX_H      15
#define GEEKO_CROUCH_Y   6
#define GEEKO_CROUCH_H   10

// Jump if standing and A or Up was pressed this frame (plays the jump sound). Returns 1 if it did.
unsigned char geeko_jump(struct Geeko *g, unsigned char pad_new);

// Vertical speed for this frame: a jump button let go early cuts the jump, gravity, fall speed cap;
// standing keeps vy at 0. Call before moving g->y.
void geeko_fall(struct Geeko *g, unsigned char pad);

// Horizontal speed: dir -1 / 0 / +1. Inertia on the ground (accelerate, skid, glide), weaker
// steering in the air. Sets facing. Not used by the race (Geeko's x is fixed there).
void geeko_run(struct Geeko *g, signed char dir);

void geeko_box(const struct Geeko *g, unsigned char crouched, struct Box *box);

// 1 while Geeko blinks out after a hit (invulnerable): skip drawing then.
unsigned char geeko_hidden(void);

// Riding pose, or the crouched one (race: duck; room: hacking), facing the right way.
void geeko_draw(const struct Geeko *g, unsigned char crouched, unsigned char y_offset);

#endif // GEEKO_H
