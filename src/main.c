/*
 * Hack to the Future: Geeko McFly
 * OpenSUSE Hack Week 26 - Retro NES race and hack
 *
 * Built with llvm-mos (mos-nes-nrom-clang), neslib, and nesdoug.
 *
 * main.c owns the game state machine; everything else lives in a module (see AGENTS.md):
 *   player / obstacles / progress  the race (Geeko on the road, score, clock)
 *   hero / room                    a hacking room (Geeko on foot, terminals, foes)
 *   render / hud / screens         what is on screen
 *   sfx                            music and sound effects (FamiTone2)
 */

#include <neslib.h>
#include <nesdoug.h>

#include "hero.h"
#include "hud.h"
#include "obstacles.h"
#include "palettes.h"
#include "player.h"
#include "progress.h"
#include "render.h"
#include "room.h"
#include "screens.h"
#include "sfx.h"
#include "tuning.h"

// Keep in step with tests/lib.lua (T.STATE).
#define STATE_TITLE        0
#define STATE_PLAYING      1   // the race
#define STATE_GAMEOVER     2   // the clock ran out
#define STATE_WARP         3   // flash at 88 MPH, the year rolls, the room appears
#define STATE_ROOM         4
#define STATE_RESULT       5   // the card after a room

#define CRASH_POSE_FRAMES  20   // the crash pose and burst last this long after a hit
#define WARP_FLASH_FRAMES  30   // brightening; then as long again (plus a bit) to fade back

static unsigned char game_state = STATE_TITLE;
static unsigned char in_room_scene;   // sprites of a room, not of the race
static unsigned char warp_timer;
static unsigned char room_id;         // only room 0 exists so far

// A fresh race: playfield, Geeko, obstacles, 20 MPH. Score and clock carry over.
static void race_begin(void) {
    player_reset();
    race_start();
    obstacles_reset();
    in_room_scene = 0;

    ppu_off();
    pal_bg(bg_palette);
    background_draw(NTADR_A(0, 0));
    background_draw(NTADR_B(0, 0)); // Second nametable for seamless scroll
    hud_draw_static(0);
    ppu_on_all();
}

// A new game: score 0, full clock, first race, music.
static void session_start(void) {
    progress_reset();
    race_begin();
    sfx_music_start();
    sfx_beep();
}

static void room_begin(void) {
    year_idx = 1;
    room_load(room_id);
    ppu_on_all();
    in_room_scene = 1;
}

static void game_over_begin(void) {
    sfx_music_stop();
    game_state = STATE_GAMEOVER;
    progress_game_over();
    game_over_show();
}

static void state_title(unsigned char pad_new) {
    if (pad_new & (PAD_START | PAD_A)) {
        seed_rng();
        session_start();
        game_state = STATE_PLAYING;
    }
#if DEV_SELECT_ROOM
    else if (pad_new & PAD_SELECT) {   // playtest: straight into the room
        seed_rng();
        progress_reset();
        sfx_music_start();
        room_begin();
        game_state = STATE_ROOM;
    }
#endif
}

static void state_playing(unsigned char pad, unsigned char pad_new) {
    struct Box player_box;

    player_update(pad, pad_new);
    progress_update();
    obstacles_update();

    // A hit slows Geeko and costs time; it never ends the run. No second hit while invulnerable.
    if (invuln == 0) {
        player_get_box(&player_box);
        if (obstacles_hit(&player_box)) {
            progress_crash();
            sfx_crash();
        }
    }

    if (!clock_tick()) {
        game_over_begin();
    } else if (mph >= MPH_WARP) {
        game_state = STATE_WARP;
        warp_timer = 0;
        sfx_warp();
    }

    hud_update(0, 0, 0);
}

static void state_warp(void) {
    ++warp_timer;
    if (warp_timer < WARP_FLASH_FRAMES) {
        pal_bright(4 + warp_timer / 8);
        if ((warp_timer & 3) == 0) hud_year_show(1985 + warp_timer * 5 / WARP_FLASH_FRAMES);
    } else if (warp_timer == WARP_FLASH_FRAMES) {
        pal_bright(8);
        room_begin();
    } else {
        pal_bright(8 - (warp_timer - WARP_FLASH_FRAMES) / 8);
        if (warp_timer >= WARP_FLASH_FRAMES + 32) {
            pal_bright(4);
            game_state = STATE_ROOM;
        }
    }
}

static void state_room(unsigned char pad, unsigned char pad_new) {
    hero_update(pad, pad_new, room_hacking());
    room_update(pad);

    if (!clock_tick()) {
        game_over_begin();
    } else if (room_done()) {
        if (room_clean()) score_add(CLEAN_ROOM_POINTS);
        result_card_draw(room_clean(), room_hacked(), room_total());
        game_state = STATE_RESULT;
    }

    hud_update(1, room_hacked(), room_total());
}

static void state_result(unsigned char pad_new) {
    if (pad_new & (PAD_START | PAD_A)) {
        year_idx = 0;       // draft: only one room, so the next race starts over in 1985
        race_begin();
        game_state = STATE_PLAYING;
    }
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

        // Sprite 0 hit split: HUD stays put, the ground below scrolls (only while racing).
        hud_scroll(game_state == STATE_PLAYING);

        pad = pad_poll(0);
        pad_new = get_pad_new(0);

        switch (game_state) {
        case STATE_TITLE:    state_title(pad_new); break;
        case STATE_PLAYING:  state_playing(pad, pad_new); break;
        case STATE_WARP:     state_warp(); break;
        case STATE_ROOM:     state_room(pad, pad_new); break;
        case STATE_RESULT:   state_result(pad_new); break;
        default:             state_gameover(pad_new); break;
        }

        if (game_state == STATE_RESULT) {
            render_clear();
        } else if (in_room_scene) {
            render_room_sprites(game_state == STATE_ROOM && room_hacking());
        } else {
            render_all_sprites(game_state == STATE_PLAYING && invuln > INVULN_FRAMES - CRASH_POSE_FRAMES);
        }
    }
}
