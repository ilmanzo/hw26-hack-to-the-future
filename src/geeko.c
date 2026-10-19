#include <neslib.h>
#include <nesdoug.h>

#include "geeko.h"
#include "progress.h"
#include "sfx.h"
#include "sprites.h"

unsigned char geeko_jump(struct Geeko *g, unsigned char pad_new) {
    if (g->grounded && (pad_new & (PAD_A | PAD_UP))) {
        g->vy = -GEEKO_JUMP;
        g->grounded = 0;
        sfx_jump();
        return 1;
    }
    return 0;
}

void geeko_fall(struct Geeko *g, unsigned char pad) {
    if (!(pad & (PAD_A | PAD_UP)) && g->vy < -GEEKO_JUMP_CUT) {
        g->vy = -GEEKO_JUMP_CUT;
    }
    if (g->grounded && g->vy >= 0) {
        g->vy = 0;
    } else {
        g->vy += GEEKO_GRAVITY;
        if (g->vy > GEEKO_FALL_MAX) g->vy = GEEKO_FALL_MAX;
    }
}

void geeko_run(struct Geeko *g, signed char dir) {
    if (dir) {
        g->facing_left = dir < 0;
        if (g->grounded) {
            g->vx += dir * ((g->vx && ((g->vx < 0) != (dir < 0))) ? GEEKO_SKID : GEEKO_ACCEL);
        } else {
            g->vx += dir * GEEKO_AIR;
        }
        if (g->vx > GEEKO_RUN_MAX) g->vx = GEEKO_RUN_MAX;
        if (g->vx < -GEEKO_RUN_MAX) g->vx = -GEEKO_RUN_MAX;
    } else if (g->grounded) {
        if (g->vx > GEEKO_FRICTION) g->vx -= GEEKO_FRICTION;
        else if (g->vx < -GEEKO_FRICTION) g->vx += GEEKO_FRICTION;
        else g->vx = 0;
    }
}

void geeko_box(const struct Geeko *g, unsigned char crouched, struct Box *box) {
    box->x = (g->x >> 8) + GEEKO_BOX_X;   // leaves the tail curl and snout tip out
    box->width = GEEKO_BOX_W;
    if (crouched) {
        box->y = (g->y >> 8) + GEEKO_CROUCH_Y;
        box->height = GEEKO_CROUCH_H;     // low profile: body rows 5..9 + board
    } else {
        box->y = (g->y >> 8) + GEEKO_BOX_Y;
        box->height = GEEKO_BOX_H;        // full standing profile: body + board
    }
}

unsigned char geeko_hidden(void) {
    return invuln && ((invuln >> 2) & 1);
}

void geeko_draw(const struct Geeko *g, unsigned char crouched, unsigned char y_offset) {
    const unsigned char *spr;

    if (crouched) {
        spr = g->facing_left ? GeekoDuckL : GeekoDuck;
    } else {
        spr = g->facing_left ? GeekoRideL : GeekoRide;
    }
    oam_meta_spr(g->x >> 8, (g->y >> 8) + y_offset, spr);
}
