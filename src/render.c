#include <neslib.h>
#include <nesdoug.h>

#include "game.h"
#include "hud.h"
#include "obstacles.h"
#include "player.h"
#include "progress.h"
#include "render.h"
#include "sprites.h"

static unsigned char fire_trail_frame = 0;

// 88 MPH fire trail behind the hoverboard; the second flame flickers.
static void fire_trail_draw(void) {
    unsigned char py = player_y();

    ++fire_trail_frame;
    oam_meta_spr(GEEKO_X - 12, py + 14, SprFireTrail);
    if (fire_trail_frame & 2) {
        oam_meta_spr(GEEKO_X - 20, py + 15, SprFireTrail);
    }
}

void render_all_sprites(unsigned char crashed) {
    oam_clear();
    hud_sprite_zero();   // must be OAM slot 0
    oam_set(4);          // Start user sprites at OAM index 4

    // OAM order = priority. The PPU drops sprites past 8 per scanline, lowest priority first.
    // Geeko is 3 tiles wide, so 2 obstacles (2 tiles each) + Geeko + fire trail can exceed 8:
    // draw the trail last so the cosmetic sprite is the one that gets dropped.
    obstacles_draw();
    player_draw(crashed);
    if (mph >= MPH_WARP) {
        fire_trail_draw();
    }
}
