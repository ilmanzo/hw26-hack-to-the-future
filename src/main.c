/*
 * Hack to the Future: Geeko McFly
 * OpenSUSE Hack Week 26 - Retro NES race and hack
 *
 * Built with llvm-mos (mos-nes-mmc3-clang), neslib, and nesdoug. Mapper: MMC3 (CHR banks, and the
 * scanline IRQ that splits the HUD from the scrolling road).
 *
 * main.c owns the game state machine; everything else lives in a module (see AGENTS.md):
 *   player / obstacles / progress  the race (Geeko on the road, score, clock)
 *   hero / room                    a hacking room (Geeko on foot, terminals, foes)
 *   render / hud / screens         what is on screen
 *   sfx                            music and sound effects (FamiTone2)
 */

#include <6502.h>
#include <neslib.h>
#include <nesdoug.h>
#include <mapper.h>

#include "gfx.h"
#include "hero.h"
#include "hud.h"
#include "map.h"
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
#define STATE_MAP          6   // the time map before each level

#define SCENE_RACE         0   // which sprites to draw
#define SCENE_ROOM         1
#define SCENE_BLANK        2   // none: cards, the ending

#define WARP_FLASH_FRAMES  30   // brightening; then as long again (plus a bit) to fade back

// ROM image: 64 KB of PRG (the MMC3 linker script needs more than 32), one 8 KB tileset.
MAPPER_PRG_ROM_KB(64);
MAPPER_CHR_ROM_KB(8);

static unsigned char game_state = STATE_TITLE;
static unsigned char scene;           // SCENE_*
static unsigned char warp_timer;
static unsigned char map_timer;
static unsigned char map_picked;      // the level-select cheat moved the selection: no timeout

// A fresh race: playfield, Geeko, obstacles, 20 MPH. Score and clock carry over.
static void race_begin(void) {
    player_reset();
    race_start();
    obstacles_reset();
    scene = SCENE_RACE;
    year_idx = epoch;

    gfx_screen_off();
    pal_bg(bg_palette);
    background_draw(NTADR_A(0, 0));
    background_draw(NTADR_B(0, 0)); // Second nametable for seamless scroll
    hud_draw_static(0);
    ppu_on_all();
}

// The time map for the era about to start; the race follows.
static void map_begin(void) {
    year_idx = epoch;
    map_timer = 0;
    map_picked = 0;
    scene = SCENE_BLANK;
    map_show(epoch);
    game_state = STATE_MAP;
}

// A new game: score 0, full clock, the map, music.
static void session_start(void) {
    progress_reset();
    sfx_music_start();
    sfx_beep();
    map_begin();
}

static void room_begin(void) {
    year_idx = epoch + 1;
    room_load(epoch);
    ppu_on_all();
    scene = SCENE_ROOM;
}

static void game_over_begin(unsigned char finished) {
    sfx_music_stop();
    game_state = STATE_GAMEOVER;
    progress_game_over();
    game_over_show(finished);
}

static void state_title(unsigned char pad_new) {
    if (pad_new & (PAD_START | PAD_A)) {
        seed_rng();
        session_start();
    }
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
        game_over_begin(0);
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
        if ((warp_timer & 3) == 0) {
            unsigned int from = year_value(epoch), to = year_value(epoch + 1);
            hud_year_show(from + (to - from) * warp_timer / WARP_FLASH_FRAMES);
        }
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
        game_over_begin(0);
    } else if (room_done()) {
        if (room_clean()) score_add(CLEAN_ROOM_POINTS);
        result_card_draw(room_clean(), room_hacked(), room_total());
        game_state = STATE_RESULT;
    }

    hud_update(1, room_hacked(), room_total());
}

static void state_result(unsigned char pad_new) {
    if (pad_new & (PAD_START | PAD_A)) {
        ++epoch;            // the next era; epoch 4 is the boss
        map_begin();
    }
}

// Leave the map for the era `epoch`: its race, or (the cheat) its room. The boss is not built yet:
// its map ends the game with TO BE CONTINUED.
static void map_leave(unsigned char to_room) {
    if (epoch >= 4) {
        scene = SCENE_BLANK;
        game_over_begin(1);
    } else if (to_room) {
        room_begin();
        game_state = STATE_ROOM;
    } else {
        race_begin();
        game_state = STATE_PLAYING;
    }
}

// The map stays MAP_FRAMES (A or Start skips it after MAP_SKIP_FRAMES), then the race starts. With
// DEV_LEVEL_SELECT, LEFT/RIGHT move the selection (and stop the timeout) and SELECT starts the room.
static void state_map(unsigned char pad_new) {
    if (map_timer < 255) ++map_timer;     // stays on the map as long as the cheat's picker wants
    hud_update(0, 0, 0);
#if DEV_LEVEL_SELECT
    if (pad_new & (PAD_LEFT | PAD_RIGHT)) {
        if ((pad_new & PAD_RIGHT) && epoch < 4) ++epoch;
        if ((pad_new & PAD_LEFT) && epoch > 0) --epoch;
        year_idx = epoch;
        map_select(epoch);
        hud_year_show(year_value(epoch));
        map_picked = 1;
    }
    if (pad_new & PAD_SELECT) {
        map_leave(1);
        return;
    }
#endif
    if ((!map_picked && map_timer >= MAP_FRAMES) || (map_timer >= MAP_SKIP_FRAMES && (pad_new & (PAD_START | PAD_A)))) {
        map_leave(0);
    }
}

static void state_gameover(unsigned char pad_new) {
    if (pad_new & (PAD_START | PAD_A)) {
        session_start();
    }
}

// The MMC3 powers on in an undefined state. CHR: the 8 KB tileset as 1 KB units 0-7, A12 inverted so
// the background (pattern table 0) is units 0-3 as four 1 KB banks and the sprites (table 1) are
// units 4-7 as two 2 KB banks. PRG mode 0, bank 0 at $8000. Vertical mirroring: the two nametables
// sit side by side for the scroll. Then the HUD's scanline IRQ.
static void mapper_init(void) {
    static const unsigned char chr_banks[6] = { 4, 6, 0, 1, 2, 3 };   // registers 0-5
    unsigned char i;

    disable_irq();
    CLI();
    set_prg_mode(PRG_MODE_0);
    set_chr_a12_inversion(CHR_A12_INVERT);
    set_prg_8000(0);
    for (i = 0; i < 6; ++i) set_chr_bank(i, chr_banks[i]);
    set_mirroring(MIRROR_VERTICAL);
    hud_irq_init();
}

int main(void) {
    unsigned char pad;
    unsigned char pad_new;

    mapper_init();
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

        // HUD stays put, the ground below scrolls (only while racing): the MMC3 IRQ splits them.
        hud_scroll(game_state == STATE_PLAYING);

        pad = pad_poll(0);
        pad_new = get_pad_new(0);

        switch (game_state) {
        case STATE_TITLE:    state_title(pad_new); break;
        case STATE_PLAYING:  state_playing(pad, pad_new); break;
        case STATE_WARP:     state_warp(); break;
        case STATE_ROOM:     state_room(pad, pad_new); break;
        case STATE_RESULT:   state_result(pad_new); break;
        case STATE_MAP:      state_map(pad_new); break;
        default:             state_gameover(pad_new); break;
        }

        if (game_state == STATE_RESULT || (scene == SCENE_BLANK && game_state != STATE_MAP)) {
            render_clear();
        } else if (game_state == STATE_MAP) {
            render_map_sprites(epoch, map_timer);
        } else if (scene == SCENE_ROOM) {
            render_room_sprites(game_state == STATE_ROOM && room_hacking());
        } else {
            render_all_sprites(game_state == STATE_PLAYING && invuln > INVULN_FRAMES - CRASH_POSE_FRAMES);
        }
    }
}
