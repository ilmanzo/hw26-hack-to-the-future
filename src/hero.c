#include <neslib.h>
#include <nesdoug.h>

#include "hero.h"
#include "progress.h"
#include "room.h"
#include "sfx.h"
#include "sprites.h"

#define H_ACCEL     0x18    // 8.8 fixed point, per frame
#define H_AIR       0x0c
#define H_MAX       0x200
#define H_FRICTION  0x10
#define H_SKID      0x30
#define H_GRAVITY   0x38
#define H_JUMP      0x4c0
#define H_JUMP_CUT  0x180   // upward speed after letting go of the jump button while rising
#define H_FALL_MAX  0x700   // under one tile per frame: no tunnelling through a 1-tile platform
#define H_HURT_VX   0x200
#define H_HURT_VY   0x300
#define H_HURT_FRAMES 10

// Hitbox inside the 16x16 sprite, as in the race: x+2, w 12; y+1, h 15.
#define BOX_X       2
#define BOX_W       12
#define BOX_Y       1
#define BOX_H       15

static unsigned int hx, hy;        // 8.8 pixels, sprite top-left
static int hvx, hvy;
static unsigned char facing_left;
static unsigned char grounded;
static unsigned char hurt_timer;   // knocked back: D-pad ignored

static unsigned char solid_column(unsigned char px, unsigned char top, unsigned char bottom) {
    unsigned char ty;
    for (ty = top >> 3; ty <= bottom >> 3; ++ty) {
        if (room_solid(px, ty << 3)) return 1;
    }
    return 0;
}

static unsigned char solid_row(unsigned char left, unsigned char right, unsigned char py) {
    return room_solid(left, py) || room_solid((left + right) >> 1, py) || room_solid(right, py);
}

void hero_reset(unsigned char row, unsigned char col) {
    hx = (unsigned int)col * 8 << 8;
    hy = (unsigned int)(row * 8 - 16) << 8;
    hvx = hvy = 0;
    facing_left = 0;
    grounded = 1;
    hurt_timer = 0;
}

static void move_x(void) {
    unsigned int nx = hx + (unsigned int)hvx;
    unsigned char px = nx >> 8;
    unsigned char top = (hy >> 8) + BOX_Y, bottom = (hy >> 8) + BOX_Y + BOX_H - 1;

    if (hvx > 0) {
        if (px > 240) { px = 240; hvx = 0; nx = (unsigned int)px << 8; }
        else if (solid_column(px + BOX_X + BOX_W - 1, top, bottom)) {
            px = ((px + BOX_X + BOX_W - 1) & 0xf8) - (BOX_X + BOX_W);
            nx = (unsigned int)px << 8;
            hvx = 0;
        }
    } else if (hvx < 0) {
        if (px > 240) { px = 0; hvx = 0; nx = 0; }   // wrapped below 0
        else if (solid_column(px + BOX_X, top, bottom)) {
            px = (((px + BOX_X) & 0xf8) + 8) - BOX_X;
            nx = (unsigned int)px << 8;
            hvx = 0;
        }
    }
    hx = nx;
}

static void move_y(void) {
    unsigned int ny = hy + (unsigned int)hvy;
    unsigned char py = ny >> 8;
    unsigned char left = (hx >> 8) + BOX_X, right = (hx >> 8) + BOX_X + BOX_W - 1;

    if (hvy > 0) {
        if (solid_row(left, right, py + BOX_Y + BOX_H - 1)) {
            py = (((py + BOX_Y + BOX_H - 1) & 0xf8)) - 16;
            ny = (unsigned int)py << 8;
            hvy = 0;
        }
    } else if (hvy < 0) {
        if (solid_row(left, right, py + BOX_Y)) {
            py = (((py + BOX_Y) & 0xf8) + 8) - BOX_Y;
            ny = (unsigned int)py << 8;
            hvy = 0;
        }
    }
    hy = ny;
}

void hero_update(unsigned char pad, unsigned char pad_new, unsigned char hacking) {
    signed char dir = 0;
    unsigned char left, right;

    if (hurt_timer) {
        --hurt_timer;
    } else if (!hacking) {
        if (pad & PAD_LEFT) dir = -1;
        else if (pad & PAD_RIGHT) dir = 1;
    }

    if (!hurt_timer) {
        if (dir) {
            facing_left = dir < 0;
            if (grounded) {
                hvx += dir * ((hvx && ((hvx < 0) != (dir < 0))) ? H_SKID : H_ACCEL);
            } else {
                hvx += dir * H_AIR;
            }
            if (hvx > H_MAX) hvx = H_MAX;
            if (hvx < -H_MAX) hvx = -H_MAX;
        } else if (grounded) {
            if (hvx > H_FRICTION) hvx -= H_FRICTION;
            else if (hvx < -H_FRICTION) hvx += H_FRICTION;
            else hvx = 0;
        }
    }

    if (grounded && !hacking && (pad_new & (PAD_A | PAD_UP))) {
        hvy = -H_JUMP;
        grounded = 0;
        sfx_jump();
    }
    if (!(pad & (PAD_A | PAD_UP)) && hvy < -H_JUMP_CUT) {
        hvy = -H_JUMP_CUT;    // letting go early: shorter jump
    }

    if (grounded && hvy >= 0) {
        hvy = 0;
    } else {
        hvy += H_GRAVITY;
        if (hvy > H_FALL_MAX) hvy = H_FALL_MAX;
    }

    move_x();
    move_y();

    // Standing on something? (a pixel row below the feet)
    left = (hx >> 8) + BOX_X;
    right = (hx >> 8) + BOX_X + BOX_W - 1;
    grounded = hvy >= 0 && solid_row(left, right, (hy >> 8) + 16);
}

void hero_knock(unsigned char foe_x) {
    hurt_timer = H_HURT_FRAMES;
    hvx = (hx >> 8) + 8 < foe_x ? -H_HURT_VX : H_HURT_VX;
    hvy = -H_HURT_VY;
    grounded = 0;
}

void hero_get_box(struct Box *box) {
    box->x = (hx >> 8) + BOX_X;
    box->y = (hy >> 8) + BOX_Y;
    box->width = BOX_W;
    box->height = BOX_H;
}

unsigned char hero_x(void) { return hx >> 8; }
unsigned char hero_y(void) { return hy >> 8; }
unsigned char hero_grounded(void) { return grounded; }
unsigned char hero_still(void) { return hvx == 0; }

void hero_draw(unsigned char hacking) {
    const unsigned char *spr;

    if (invuln && ((invuln >> 2) & 1)) {
        return;     // blink while invulnerable
    }
    if (hacking) {
        spr = facing_left ? GeekoDuckL : GeekoDuck;
    } else {
        spr = facing_left ? GeekoRideL : GeekoRide;
    }
    oam_meta_spr(hx >> 8, hy >> 8, spr);
}
