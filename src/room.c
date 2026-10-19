#include <neslib.h>
#include <nesdoug.h>

#include "game.h"
#include "hero.h"
#include "hud.h"
#include "palettes.h"
#include "progress.h"
#include "room.h"
#include "rooms.h"
#include "sfx.h"
#include "sprites.h"
#include "tuning.h"

// Background tiles (pattern table 0, see tools/build_chr.py)
#define TILE_BAR         0x01    // 0x01..0x09: progress bar, 0..8 pixels filled
#define TILE_TERM_IDLE   0x90    // 2x2
#define TILE_TERM_DONE   0x94
#define TILE_GATE_SHUT   0x98    // 2x3
#define TILE_GATE_OPEN   0x9e
#define TILE_SOLID_TOP   0xc0
#define TILE_SOLID_BODY  0xc1

#define MAX_TERMS        6
#define MAX_FOES         4

static const unsigned char hack_frames_by_room[N_ROOMS] = { HACK_FRAMES_1990 };

static const struct RoomDef *room;
static unsigned char hack_frames;
static unsigned char term_prog[MAX_TERMS];     // frames held so far
static unsigned char term_done[MAX_TERMS];
static unsigned char term_fill[MAX_TERMS];     // bar pixels drawn (0..32)
static unsigned char n_hacked;
static unsigned char hacking;
static unsigned char gate_open;
static unsigned char finished;
static unsigned char hit_taken;

// Foes (drones): position 8.8, patrol limits in pixels, direction.
static unsigned int foe_x[MAX_FOES];
static unsigned char foe_y[MAX_FOES];
static unsigned int foe_min[MAX_FOES], foe_max[MAX_FOES];
static signed char foe_dir[MAX_FOES];
#define DRONE_SPEED      0x80      // 0.5 px/frame

static unsigned char attr[64];

static void attr_set(unsigned char col, unsigned char row, unsigned char pal) {
    unsigned char i = (row >> 2) * 8 + (col >> 2);
    unsigned char shift = ((row >> 1) & 1) * 4 + ((col >> 1) & 1) * 2;
    attr[i] = (attr[i] & ~(3 << shift)) | (pal << shift);
}

unsigned char room_solid(unsigned char x, unsigned char y) {
    unsigned char r = y >> 3, c = x >> 3, i;

    if (r >= 30) return 0;
    for (i = 0; i < room->n_rects; ++i) {
        const struct Rect *q = &room->rects[i];
        if (r >= q->r0 && r <= q->r1 && c >= q->c0 && c <= q->c1) return 1;
    }
    return 0;
}

static void draw_block(unsigned char col, unsigned char row, unsigned char first, unsigned char w, unsigned char h) {
    unsigned char x, y, t = first;
    for (y = 0; y < h; ++y) {
        vram_adr(NTADR_A(col, row + y));
        for (x = 0; x < w; ++x) vram_put(t++);
    }
}

void room_load(unsigned char id) {
    unsigned char i, y;

    room = &room_defs[id];
    hack_frames = hack_frames_by_room[id];
    n_hacked = hacking = gate_open = finished = 0;
    hit_taken = 0;
    progress_room_start();

    ppu_off();
    pal_bg(room_bg_palette);

    vram_adr(NTADR_A(0, 0));
    vram_fill(0, 1024);                      // blank tiles and attributes (palette 0)

    for (i = 0; i < 64; ++i) attr[i] = 0;

    for (i = 0; i < room->n_rects; ++i) {
        const struct Rect *q = &room->rects[i];
        for (y = q->r0; y <= q->r1; ++y) {
            vram_adr(NTADR_A(q->c0, y));
            vram_fill(y == q->r0 ? TILE_SOLID_TOP : TILE_SOLID_BODY, q->c1 - q->c0 + 1);
        }
    }

    for (i = 0; i < room->n_terms; ++i) {
        unsigned char row = room->terms[i * 2], col = room->terms[i * 2 + 1];
        term_prog[i] = term_done[i] = term_fill[i] = 0;
        draw_block(col, row - 2, TILE_TERM_IDLE, 2, 2);
        attr_set(col, row - 2, 1); attr_set(col + 1, row - 2, 1);
        attr_set(col, row - 1, 1); attr_set(col + 1, row - 1, 1);
    }

    draw_block(room->gate_col, room->gate_row - 3, TILE_GATE_SHUT, room->gate_w, 3);
    for (y = room->gate_row - 3; y < room->gate_row; ++y) {
        for (i = 0; i < room->gate_w; ++i) attr_set(room->gate_col + i, y, 2);
    }

    vram_adr(0x23c0);
    vram_write(attr, 64);

    hud_draw_static(1);

    for (i = 0; i < room->n_foes; ++i) {
        const struct Foe *f = &room->foes[i];
        foe_min[i] = (unsigned int)f->c0 * 8 << 8;
        foe_max[i] = (unsigned int)((f->c1 + 1) * 8 - 16) << 8;
        foe_x[i] = foe_min[i];
        foe_y[i] = f->row * 8;
        foe_dir[i] = 1;
    }

    hero_reset(room->start_row, room->start_col);
}

static void draw_bar(unsigned char i, unsigned char fill) {
    unsigned char row = room->terms[i * 2], col = room->terms[i * 2 + 1];
    char tiles[4];
    unsigned char k;
    int left;

    for (k = 0; k < 4; ++k) {
        left = (int)fill - k * 8;
        tiles[k] = fill == 0 ? 0 : TILE_BAR + (left < 0 ? 0 : left > 8 ? 8 : left);
    }
    multi_vram_buffer_horz(tiles, 4, NTADR_A(col - 1, row - 3));
    term_fill[i] = fill;
}

static void finish_terminal(unsigned char i) {
    unsigned char row = room->terms[i * 2], col = room->terms[i * 2 + 1];
    static const char top[2] = { TILE_TERM_DONE, TILE_TERM_DONE + 1 };
    static const char bottom[2] = { TILE_TERM_DONE + 2, TILE_TERM_DONE + 3 };
    static const char open_row[3][2] = {
        { TILE_GATE_OPEN, TILE_GATE_OPEN + 1 }, { TILE_GATE_OPEN + 2, TILE_GATE_OPEN + 3 },
        { TILE_GATE_OPEN + 4, TILE_GATE_OPEN + 5 } };
    unsigned char y;

    term_done[i] = 1;
    ++n_hacked;
    draw_bar(i, 0);
    multi_vram_buffer_horz(top, 2, NTADR_A(col, row - 2));
    multi_vram_buffer_horz(bottom, 2, NTADR_A(col, row - 1));
    clock_add(HACK_TIME_BONUS_S);
    score_add(TERMINAL_POINTS);
    sfx_warp();

    if (n_hacked == room->n_terms) {
        gate_open = 1;
        for (y = 0; y < 3; ++y) {
            multi_vram_buffer_horz(open_row[y], 2, NTADR_A(room->gate_col, room->gate_row - 3 + y));
        }
    }
}

void room_update(unsigned char pad) {
    unsigned char i, cx, foot, in_zone;
    struct Box hb, fb;

    // Terminals: hold B in the zone (the terminal's two columns plus one each side) while standing
    // on its surface; D-pad is ignored then, so Geeko slides to a stop; progress runs while still.
    cx = hero_x() + 8;
    foot = hero_y() + 16;
    hacking = 0;
    for (i = 0; i < room->n_terms; ++i) {
        unsigned char row = room->terms[i * 2], col = room->terms[i * 2 + 1];
        if (term_done[i]) continue;
        in_zone = hero_grounded() && foot == row * 8 && cx >= (col - 1) * 8 && cx < (col + 3) * 8;
        if (in_zone && (pad & PAD_B)) {
            hacking = 1;
            if (hero_still()) {
                if (++term_prog[i] >= hack_frames) {
                    finish_terminal(i);
                } else {
                    unsigned char fill = (unsigned int)term_prog[i] * 32 / hack_frames;
                    if ((term_prog[i] & 7) == 0) sfx_beep();
                    if (fill != term_fill[i]) draw_bar(i, fill);
                }
            }
        } else if (term_prog[i]) {
            term_prog[i] = 0;
            draw_bar(i, 0);
        }
    }

    // Foes: patrol, and a touch costs time (once per invulnerability).
    hero_get_box(&hb);
    for (i = 0; i < room->n_foes; ++i) {
        foe_x[i] += (int)foe_dir[i] * DRONE_SPEED;
        if (foe_x[i] >= foe_max[i]) { foe_x[i] = foe_max[i]; foe_dir[i] = -1; }
        if (foe_x[i] <= foe_min[i]) { foe_x[i] = foe_min[i]; foe_dir[i] = 1; }
        fb.x = (foe_x[i] >> 8) + 2;
        fb.y = foe_y[i] + 2;
        fb.width = 12;
        fb.height = 12;
        if (invuln == 0 && check_collision(&hb, &fb)) {
            progress_hit();
            hero_knock((foe_x[i] >> 8) + 8);
            sfx_crash();
            hit_taken = 1;
        }
    }

    // The gate: open once everything is hacked; touching it ends the room.
    if (gate_open) {
        fb.x = room->gate_col * 8;
        fb.y = (room->gate_row - 3) * 8;
        fb.width = room->gate_w * 8;
        fb.height = 24;
        if (check_collision(&hb, &fb)) finished = 1;
    }
}

void room_draw_foes(void) {
    unsigned char i;
    for (i = 0; i < room->n_foes; ++i) {
        oam_meta_spr(foe_x[i] >> 8, foe_y[i], SprDrone);
    }
}

unsigned char room_hacking(void) { return hacking; }
unsigned char room_hacked(void)  { return n_hacked; }
unsigned char room_total(void)   { return room->n_terms; }
unsigned char room_done(void)    { return finished; }
unsigned char room_clean(void)   { return !hit_taken; }
