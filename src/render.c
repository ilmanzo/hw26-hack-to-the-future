#include <neslib.h>
#include <nesdoug.h>

#include "game.h"
#include "hero.h"
#include "hud.h"
#include "map.h"
#include "obstacles.h"
#include "player.h"
#include "progress.h"
#include "render.h"
#include "room.h"
#include "sprites.h"

static unsigned char fire_trail_frame = 0;

// 88 MPH fire trail behind the hoverboard; the second flame flickers.
static void fire_trail_draw(void) {
    unsigned char py = player_y();

    ++fire_trail_frame;
    oam_meta_spr(GEEKO_X - 12, py + 7, SprFireTrail);
    if (fire_trail_frame & 2) {
        oam_meta_spr(GEEKO_X - 20, py + 8, SprFireTrail);
    }
}

// Every frame starts the same way: sprite 0 in OAM slot 0, user sprites from OAM index 4.
static void oam_begin(void) {
    oam_clear();
    hud_sprite_zero();
    oam_set(4);
}

void render_all_sprites(unsigned char crashed) {
    oam_begin();

    // OAM order = priority. The PPU drops sprites past 8 per scanline, lowest priority first.
    // Geeko is 2 tiles wide, so 2 obstacles (2 tiles each) + Geeko + fire trail stay near 8:
    // draw the trail last so the cosmetic sprite is the one that gets dropped.
    obstacles_draw();
    player_draw(crashed);
    if (mph >= MPH_WARP) {
        fire_trail_draw();
    }
}

void render_room_sprites(unsigned char hacking) {
    oam_begin();
    room_draw_foes();
    hero_draw(hacking);
}

void render_clear(void) {
    oam_clear();
    hud_sprite_zero();
}

void render_map_sprites(unsigned char epoch, unsigned char timer) {
    oam_begin();
    map_draw_geeko(epoch, timer);
}
