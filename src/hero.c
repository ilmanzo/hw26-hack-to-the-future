#include <neslib.h>
#include <nesdoug.h>

#include "geeko.h"
#include "hero.h"
#include "progress.h"
#include "room.h"
#include "sprites.h"

// Geeko in a room: the shared body (geeko.c) plus tile collision and knockback.
#define H_HURT_VX   0x200
#define H_HURT_VY   0x300
#define H_HURT_FRAMES 10

static struct Geeko hero;
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
    hero.x = (unsigned int)col * 8 << 8;
    hero.y = (unsigned int)(row * 8 - 16) << 8;
    hero.vx = hero.vy = 0;
    hero.facing_left = 0;
    hero.grounded = 1;
    hurt_timer = 0;
}

static void move_x(void) {
    unsigned int nx = hero.x + (unsigned int)hero.vx;
    unsigned char px = nx >> 8;
    unsigned char top = (hero.y >> 8) + GEEKO_BOX_Y;
    unsigned char bottom = top + GEEKO_BOX_H - 1;

    if (hero.vx > 0) {
        if (px > 240) { px = 240; hero.vx = 0; nx = (unsigned int)px << 8; }
        else if (solid_column(px + GEEKO_BOX_X + GEEKO_BOX_W - 1, top, bottom)) {
            px = ((px + GEEKO_BOX_X + GEEKO_BOX_W - 1) & 0xf8) - (GEEKO_BOX_X + GEEKO_BOX_W);
            nx = (unsigned int)px << 8;
            hero.vx = 0;
        }
    } else if (hero.vx < 0) {
        if (px > 240) { px = 0; hero.vx = 0; nx = 0; }   // wrapped below 0
        else if (solid_column(px + GEEKO_BOX_X, top, bottom)) {
            px = (((px + GEEKO_BOX_X) & 0xf8) + 8) - GEEKO_BOX_X;
            nx = (unsigned int)px << 8;
            hero.vx = 0;
        }
    }
    hero.x = nx;
}

static void move_y(void) {
    unsigned int ny = hero.y + (unsigned int)hero.vy;
    unsigned char py = ny >> 8;
    unsigned char left = (hero.x >> 8) + GEEKO_BOX_X;
    unsigned char right = left + GEEKO_BOX_W - 1;

    if (hero.vy > 0) {
        if (solid_row(left, right, py + GEEKO_BOX_Y + GEEKO_BOX_H - 1)) {
            py = ((py + GEEKO_BOX_Y + GEEKO_BOX_H - 1) & 0xf8) - 16;
            ny = (unsigned int)py << 8;
            hero.vy = 0;
        }
    } else if (hero.vy < 0) {
        if (solid_row(left, right, py + GEEKO_BOX_Y)) {
            py = (((py + GEEKO_BOX_Y) & 0xf8) + 8) - GEEKO_BOX_Y;
            ny = (unsigned int)py << 8;
            hero.vy = 0;
        }
    }
    hero.y = ny;
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

    // Run, jump and fall: the same code as in the race (geeko.c)
    if (!hurt_timer) geeko_run(&hero, dir);
    if (!hacking) geeko_jump(&hero, pad_new);
    geeko_fall(&hero, pad);

    move_x();
    move_y();

    // Standing on something? (a pixel row below the feet)
    left = (hero.x >> 8) + GEEKO_BOX_X;
    right = left + GEEKO_BOX_W - 1;
    hero.grounded = hero.vy >= 0 && solid_row(left, right, (hero.y >> 8) + 16);
}

void hero_knock(unsigned char foe_x) {
    hurt_timer = H_HURT_FRAMES;
    hero.vx = (hero.x >> 8) + 8 < foe_x ? -H_HURT_VX : H_HURT_VX;
    hero.vy = -H_HURT_VY;
    hero.grounded = 0;
}

void hero_get_box(struct Box *box) {
    geeko_box(&hero, 0, box);
}

unsigned char hero_x(void) { return hero.x >> 8; }
unsigned char hero_y(void) { return hero.y >> 8; }
unsigned char hero_grounded(void) { return hero.grounded; }
unsigned char hero_still(void) { return hero.vx == 0; }

void hero_draw(unsigned char hacking) {
    if (geeko_hidden()) {
        return;     // blink while invulnerable
    }
    geeko_draw(&hero, hacking, 0);
}
