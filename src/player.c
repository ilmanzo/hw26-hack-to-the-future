#include <neslib.h>
#include <nesdoug.h>

#include "player.h"
#include "progress.h"
#include "sfx.h"
#include "sprites.h"

#define GRAVITY            0x38     // 8.8 fixed-point gravity
#define JUMP_IMPULSE       (-0x4c0) // Initial jump velocity
#define GROUND_Y_SUB       ((unsigned int)GROUND_Y << 8)

// Y position is 8.8 fixed point; the integer part is the screen Y.
static unsigned int player_y_sub = GROUND_Y_SUB;
static int player_vel_y = 0;
static unsigned char player_jumping = 0;
static unsigned char player_ducking = 0;
static unsigned char player_bob_timer = 0;

// Crouched only counts on the ground: a ducking player who jumps stands up.
static unsigned char is_crouched(void) {
    return player_ducking && !player_jumping;
}

void player_reset(void) {
    player_y_sub = GROUND_Y_SUB;
    player_vel_y = 0;
    player_jumping = 0;
    player_ducking = 0;
    player_bob_timer = 0;
}

void player_update(unsigned char pad, unsigned char pad_new) {
    ++player_bob_timer;

    // Jump input
    if ((pad_new & (PAD_A | PAD_UP)) && !player_jumping) {
        player_jumping = 1;
        player_vel_y = JUMP_IMPULSE;
        sfx_jump();
    }

    // Duck input
    if (pad & PAD_DOWN) {
        if (!player_ducking && !player_jumping) {
            sfx_duck();
        }
        player_ducking = 1;
    } else {
        player_ducking = 0;
    }

    // Jump physics
    if (player_jumping) {
        player_vel_y += GRAVITY;
        player_y_sub += player_vel_y;

        // Land on ground
        if (player_y_sub >= GROUND_Y_SUB) {
            player_y_sub = GROUND_Y_SUB;
            player_vel_y = 0;
            player_jumping = 0;
        }
    }
}

unsigned char player_y(void) {
    return player_y_sub >> 8;
}

void player_get_box(struct Box *box) {
    unsigned char py = player_y();

    box->x = GEEKO_X + 2;  // sprite spans x 0..15; box leaves the tail curl and snout tip out
    box->width = 12;

    if (is_crouched()) {
        box->y = py + 6;
        box->height = 10; // Low crouch profile: body rows 5..9 + board
    } else {
        box->y = py + 1;
        box->height = 15; // Full standing profile: body + board
    }
}

void player_draw(unsigned char crashed) {
    unsigned char py = player_y();

    if (!crashed && invuln && ((invuln >> 2) & 1)) {
        return;     // blink while invulnerable
    }

    if (crashed) {
        // Crash burst over Geeko
        oam_meta_spr(GEEKO_X + 4, py + 2, SprCrash);
        oam_meta_spr(GEEKO_X, py, GeekoDuck);
    } else if (is_crouched()) {
        oam_meta_spr(GEEKO_X, py, GeekoDuck);
    } else {
        // Add subtle hoverboard floating bob
        unsigned char bob = player_jumping ? 0 : ((player_bob_timer >> 3) & 1);
        oam_meta_spr(GEEKO_X, py + bob, GeekoRide);
    }
}
