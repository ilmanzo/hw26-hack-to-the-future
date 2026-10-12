#include <neslib.h>
#include <nesdoug.h>

#include "obstacles.h"
#include "progress.h"
#include "sprites.h"

#define MAX_OBSTACLES      2

#define OBS_BUG            0
#define OBS_TUMBLEWEED     1
#define OBS_DRONE          2

// First four fields must stay x, y, width, height: this struct is passed to nesdoug's
// check_collision() through struct Box (see game.h).
struct Obstacle {
    unsigned char x;
    unsigned char y;
    unsigned char width;
    unsigned char height;
    unsigned char type;
    unsigned char active;
};

static struct Obstacle obstacles[MAX_OBSTACLES];
static unsigned char spawn_timer = 0;   // frames until the next spawn

void obstacles_reset(void) {
    unsigned char i;

    spawn_timer = 40;
    for (i = 0; i < MAX_OBSTACLES; ++i) {
        obstacles[i].active = 0;
    }
}

static void spawn(struct Obstacle *o) {
    unsigned char r = rand8() % 3;

    o->active = 1;
    o->x = 240;
    o->type = r;
    o->width = 12;

    if (r == OBS_DRONE) {
        o->y = 136; // Flying height: must duck!
        o->height = 10;
    } else {
        o->y = 154; // Ground height: must jump!
        o->height = 14;
    }

    // Random gap to next spawn
    spawn_timer = 50 + (rand8() % 40) - (speed_px * 4);
    if (spawn_timer < 35) spawn_timer = 35;
}

void obstacles_update(void) {
    unsigned char i;

    // Spawn countdown
    if (spawn_timer > 0) {
        --spawn_timer;
    } else {
        // Find inactive slot
        for (i = 0; i < MAX_OBSTACLES; ++i) {
            if (!obstacles[i].active) {
                spawn(&obstacles[i]);
                break;
            }
        }
    }

    // Move active obstacles
    for (i = 0; i < MAX_OBSTACLES; ++i) {
        if (obstacles[i].active) {
            if (obstacles[i].x <= speed_px) {
                obstacles[i].active = 0;
            } else {
                obstacles[i].x -= speed_px;
            }
        }
    }
}

unsigned char obstacles_hit(const struct Box *box) {
    unsigned char i;

    for (i = 0; i < MAX_OBSTACLES; ++i) {
        if (obstacles[i].active) {
            struct Box obox;
            obox.x = obstacles[i].x + 2;
            obox.y = obstacles[i].y + 2;
            obox.width = obstacles[i].width;
            obox.height = obstacles[i].height;

            if (check_collision((void *)box, &obox)) {
                return 1;
            }
        }
    }
    return 0;
}

void obstacles_draw(void) {
    unsigned char i;

    for (i = 0; i < MAX_OBSTACLES; ++i) {
        if (obstacles[i].active) {
            if (obstacles[i].type == OBS_BUG) {
                oam_meta_spr(obstacles[i].x, obstacles[i].y, SprBug);
            } else if (obstacles[i].type == OBS_TUMBLEWEED) {
                oam_meta_spr(obstacles[i].x, obstacles[i].y, SprTumbleweed);
            } else {
                oam_meta_spr(obstacles[i].x, obstacles[i].y, SprDrone);
            }
        }
    }
}
