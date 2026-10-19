#include <neslib.h>
#include <nesdoug.h>

#include "geeko.h"
#include "player.h"
#include "progress.h"
#include "sfx.h"
#include "sprites.h"

#define GROUND_Y_SUB       ((unsigned int)GROUND_Y << 8)

// The race: Geeko's x is fixed, the road is flat. Jump, fall and hitbox come from geeko.c (shared
// with the rooms); this file only adds the ground, ducking and the hoverboard bob.
static struct Geeko player;
static unsigned char player_ducking = 0;
static unsigned char player_bob_timer = 0;

// Crouched only counts on the ground: a ducking player who jumps stands up.
static unsigned char is_crouched(void) {
    return player_ducking && player.grounded;
}

void player_reset(void) {
    player.x = (unsigned int)GEEKO_X << 8;
    player.y = GROUND_Y_SUB;
    player.vx = player.vy = 0;
    player.grounded = 1;
    player.facing_left = 0;
    player_ducking = 0;
    player_bob_timer = 0;
}

void player_update(unsigned char pad, unsigned char pad_new) {
    ++player_bob_timer;

    geeko_jump(&player, pad_new);

    // Duck input
    if (pad & PAD_DOWN) {
        if (!player_ducking && player.grounded) {
            sfx_duck();
        }
        player_ducking = 1;
    } else {
        player_ducking = 0;
    }

    geeko_fall(&player, pad);
    player.y += player.vy;

    // The road
    if (player.y >= GROUND_Y_SUB) {
        player.y = GROUND_Y_SUB;
        player.vy = 0;
        player.grounded = 1;
    } else {
        player.grounded = 0;
    }
}

unsigned char player_y(void) {
    return player.y >> 8;
}

void player_get_box(struct Box *box) {
    geeko_box(&player, is_crouched(), box);
}

void player_draw(unsigned char crashed) {
    if (crashed) {
        // Crash burst over Geeko
        oam_meta_spr(GEEKO_X + 4, player_y() + 2, SprCrash);
        geeko_draw(&player, 1, 0);
    } else if (geeko_hidden()) {
        return;     // blink while invulnerable
    } else if (is_crouched()) {
        geeko_draw(&player, 1, 0);
    } else {
        // Add subtle hoverboard floating bob
        geeko_draw(&player, 0, player.grounded ? ((player_bob_timer >> 3) & 1) : 0);
    }
}
