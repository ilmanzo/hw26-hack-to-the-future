/*
 * Hack to the Future: Geeko McFly
 * OpenSUSE Hack Week 26 - Retro NES Endless Runner
 *
 * Built with llvm-mos (mos-nes-nrom-clang), neslib, and nesdoug.
 *
 * main.c owns the game state machine; everything else lives in a module (see AGENTS.md):
 *   player / obstacles / progress  game logic
 *   render / hud / screens         what is on screen
 *   sfx                            music and sound effects (FamiTone2)
 */

#include <neslib.h>
#include <nesdoug.h>

#include "hud.h"
#include "obstacles.h"
#include "palettes.h"
#include "player.h"
#include "progress.h"
#include "render.h"
#include "screens.h"
#include "sfx.h"

#define STATE_TITLE        0
#define STATE_PLAYING      1
#define STATE_GAMEOVER     2

static unsigned char game_state = STATE_TITLE;

// Reset every module and draw a fresh playfield.
static void session_start(void) {
    player_reset();
    progress_reset();
    obstacles_reset();

    ppu_off();
    background_draw(NTADR_A(0, 0));
    background_draw(NTADR_B(0, 0)); // Second nametable for seamless scroll
    hud_draw_static();
    ppu_on_all();

    sfx_music_start();
    sfx_beep();
}

static void state_title(unsigned char pad_new) {
    if (pad_new & (PAD_START | PAD_A)) {
        seed_rng();
        session_start();
        game_state = STATE_PLAYING;
    }
}

static void state_playing(unsigned char pad, unsigned char pad_new) {
    struct Box player_box;

    player_update(pad, pad_new);
    progress_update();
    obstacles_update();

    player_get_box(&player_box);
    if (obstacles_hit(&player_box)) {
        sfx_music_stop();
        sfx_crash();
        game_state = STATE_GAMEOVER;
        progress_game_over();
        crash_banner_show();
    }

    hud_update();
}

static void state_gameover(unsigned char pad_new) {
    if (pad_new & (PAD_START | PAD_A)) {
        session_start();
        game_state = STATE_PLAYING;
    }
}

int main(void) {
    unsigned char pad;
    unsigned char pad_new;

    sfx_init();
    ppu_off();

    pal_bg(bg_palette);
    pal_spr(spr_palette);

    bank_spr(1); // Use CHR pattern table 1 for sprites
    bank_bg(0);  // Use CHR pattern table 0 for background & font

    set_vram_buffer();

    title_screen_draw();

    while (1) {
        ppu_wait_nmi();

        // Sprite 0 hit split: HUD stays put, the ground below scrolls (only while playing).
        hud_scroll(game_state == STATE_PLAYING);

        pad = pad_poll(0);
        pad_new = get_pad_new(0);

        if (game_state == STATE_TITLE) {
            state_title(pad_new);
        } else if (game_state == STATE_PLAYING) {
            state_playing(pad, pad_new);
        } else {
            state_gameover(pad_new);
        }

        render_all_sprites(game_state == STATE_GAMEOVER);
    }
}
