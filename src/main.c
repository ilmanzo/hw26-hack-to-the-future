/*
 * Hack to the Future: Geeko McFly
 * OpenSUSE Hack Week 26 - Retro NES Endless Runner
 *
 * Built with llvm-mos (mos-nes-nrom-clang), neslib, and nesdoug.
 */

#include <neslib.h>
#include <nesdoug.h>
#include <peekpoke.h>

#include "palettes.h"
#include "sprites.h"

// --- CONSTANTS ---
#define GROUND_Y           148    // Geeko hoverboard ground plane
#define GEEKO_X            36     // Fixed horizontal screen position

#define GRAVITY            0x38   // 8.8 fixed-point gravity
#define JUMP_IMPULSE       (-0x4c0) // Initial jump velocity

#define MAX_OBSTACLES      2

#define OBS_BUG            0
#define OBS_TUMBLEWEED     1
#define OBS_DRONE          2

#define STATE_TITLE        0
#define STATE_PLAYING      1
#define STATE_GAMEOVER     2

// Collision box matching nesdoug check_collision signature:
// struct { unsigned char x, y, width, height; ... }
struct Box {
    unsigned char x;
    unsigned char y;
    unsigned char width;
    unsigned char height;
};

struct Obstacle {
    unsigned char x;
    unsigned char y;
    unsigned char width;
    unsigned char height;
    unsigned char type;
    unsigned char active;
};

// --- GAME STATE ---
static unsigned char game_state = STATE_TITLE;

static unsigned int player_y_sub = (GROUND_Y << 8);
static int player_vel_y = 0;
static unsigned char is_jumping = 0;
static unsigned char is_ducking = 0;
static unsigned char bob_timer = 0;

static unsigned int scroll_x = 0;
static unsigned char speed_px = 2;
static unsigned char mph = 20;

static unsigned int score = 0;
static unsigned int hi_score = 0;
static unsigned char score_sub = 0;

static struct Obstacle obstacles[MAX_OBSTACLES];
static unsigned char spawn_timer = 0;

static unsigned char pad1 = 0;
static unsigned char pad1_new = 0;
static unsigned char warp_flash_timer = 0;
static unsigned char fire_trail_frame = 0;

// --- APU SOUND EFFECTS ---
static void sfx_init(void) {
    POKE(0x4015, 0x0f); // Enable Pulse 1, 2, Triangle, Noise
}

static void sfx_jump(void) {
    POKE(0x4000, 0x98); // Pulse 1: 50% duty, decay
    POKE(0x4001, 0x82); // Sweep up
    POKE(0x4002, 0x70); // Base pitch
    POKE(0x4003, 0x98); // Trigger length
}

static void sfx_duck(void) {
    POKE(0x4000, 0x58); // Pulse 1: 25% duty, short chirp
    POKE(0x4001, 0x80);
    POKE(0x4002, 0xa0);
    POKE(0x4003, 0x98);
}

static void sfx_beep(void) {
    POKE(0x4004, 0x8c); // Pulse 2: 50% duty
    POKE(0x4005, 0x7f);
    POKE(0x4006, 0x22); // High chime (C6)
    POKE(0x4007, 0x98);
}

static void sfx_warp(void) {
    POKE(0x4008, 0x7f); // Triangle tone
    POKE(0x400a, 0x18);
    POKE(0x400b, 0x98);
}

static void sfx_crash(void) {
    POKE(0x400c, 0x3f); // Noise channel: max volume
    POKE(0x400e, 0x0e); // Low rumbling white noise
    POKE(0x400f, 0x98); // Trigger
}

// --- HELPER: NUMBER TO STRING ---
static void num_to_str5(unsigned int val, char *buf) {
    buf[0] = '0' + ((val / 10000) % 10);
    buf[1] = '0' + ((val / 1000) % 10);
    buf[2] = '0' + ((val / 100) % 10);
    buf[3] = '0' + ((val / 10) % 10);
    buf[4] = '0' + (val % 10);
}

static void num_to_str2(unsigned char val, char *buf) {
    buf[0] = '0' + ((val / 10) % 10);
    buf[1] = '0' + (val % 10);
}

// --- BACKGROUND GENERATION ---
static void setup_nametable(unsigned int base_addr) {
    unsigned char col, row;

    // Rows 0..2: Reserved for HUD
    // Row 3..14: Cyber Sky & Neon Mountains
    for (row = 3; row < 15; ++row) {
        vram_adr(base_addr + (row * 32));
        for (col = 0; col < 32; ++col) {
            if (row == 5 && (col == 6 || col == 22)) {
                vram_put(0x86); // Cloud left
            } else if (row == 5 && (col == 7 || col == 23)) {
                vram_put(0x87); // Cloud right
            } else if (row == 12 && (col % 8 == 0)) {
                vram_put(0x84); // Mountain peak left
            } else if (row == 12 && (col % 8 == 1)) {
                vram_put(0x85); // Mountain peak right
            } else if (row == 13 && (col % 8 == 7 || col % 8 == 0 || col % 8 == 1 || col % 8 == 2)) {
                vram_put(0x83); // Mountain body
            } else if (row == 14 && (col % 8 >= 6 || col % 8 <= 3)) {
                vram_put(0x83); // Mountain base
            } else {
                vram_put(0x00); // Empty sky
            }
        }
    }

    // Rows 15..20: Cyber Horizon
    for (row = 15; row < 21; ++row) {
        vram_adr(base_addr + (row * 32));
        for (col = 0; col < 32; ++col) {
            vram_put(0x00);
        }
    }

    // Row 21: Road Horizon line (Tile 0x80)
    vram_adr(base_addr + (21 * 32));
    for (col = 0; col < 32; ++col) {
        vram_put(0x80);
    }

    // Rows 22..24: Road surface with traces & center dash
    for (row = 22; row < 25; ++row) {
        vram_adr(base_addr + (row * 32));
        for (col = 0; col < 32; ++col) {
            if (row == 23 && (col % 4 < 2)) {
                vram_put(0x82); // Dashed center highway line
            } else {
                vram_put(0x81); // Circuit trace road
            }
        }
    }

    // Rows 25..29: Solid Cyber Bedrock
    for (row = 25; row < 30; ++row) {
        vram_adr(base_addr + (row * 32));
        for (col = 0; col < 32; ++col) {
            vram_put(0x83);
        }
    }
}

static void draw_title_screen(void) {
    ppu_off();
    
    // Clear Nametable 0
    vram_adr(NTADR_A(0, 0));
    vram_fill(0x00, 1024);
    
    setup_nametable(NTADR_A(0, 0));

    // Title Banners
    vram_adr(NTADR_A(7, 4));
    vram_write("HACK TO THE FUTURE", 18);

    vram_adr(NTADR_A(9, 6));
    vram_write("GEEKO MCFLY '85", 15);

    vram_adr(NTADR_A(4, 8));
    vram_write("OPENSUSE HACK WEEK 26", 21);

    vram_adr(NTADR_A(2, 11));
    vram_write("\"WHERE WE GO, NO ROADS!\"", 24);

    vram_adr(NTADR_A(8, 16));
    vram_write("PRESS START OR A", 16);

    vram_adr(NTADR_A(6, 18));
    vram_write("A: JUMP   DOWN: DUCK", 20);

    // Sprite 0 trigger tile in Nametable 0
    vram_adr(NTADR_A(30, 2));
    vram_put(0x80);

    ppu_on_all();
}

static void draw_hud_static(void) {
    // Write stationary HUD to rows 1 and 2
    vram_adr(NTADR_A(1, 1));
    vram_write("HW26: HACK TO THE FUTURE", 24);

    vram_adr(NTADR_A(1, 2));
    vram_write("SPD:20MPH  SCR:00000 HI:00000", 29);

    // Sprite 0 hit tile at row 2 col 30
    vram_adr(NTADR_A(30, 2));
    vram_put(0x80);
}

static void init_game_session(void) {
    unsigned char i;

    player_y_sub = (GROUND_Y << 8);
    player_vel_y = 0;
    is_jumping = 0;
    is_ducking = 0;
    bob_timer = 0;

    scroll_x = 0;
    speed_px = 2;
    mph = 20;

    score = 0;
    score_sub = 0;
    spawn_timer = 40;
    warp_flash_timer = 0;

    for (i = 0; i < MAX_OBSTACLES; ++i) {
        obstacles[i].active = 0;
    }

    ppu_off();
    setup_nametable(NTADR_A(0, 0));
    setup_nametable(NTADR_B(0, 0)); // Second Nametable for seamless scroll
    draw_hud_static();
    ppu_on_all();

    sfx_beep();
}

// --- UPDATE HUD VIA VRAM BUFFER ---
static void update_hud_vram(void) {
    static char buf_spd[2];
    static char buf_scr[5];
    static char buf_hi[5];

    num_to_str2(mph, buf_spd);
    multi_vram_buffer_horz(buf_spd, 2, NTADR_A(5, 2));

    num_to_str5(score, buf_scr);
    multi_vram_buffer_horz(buf_scr, 5, NTADR_A(15, 2));

    num_to_str5(hi_score, buf_hi);
    multi_vram_buffer_horz(buf_hi, 5, NTADR_A(25, 2));
}

// --- OBSTACLES MANAGEMENT ---
static void update_obstacles(void) {
    unsigned char i;

    // Spawn countdown
    if (spawn_timer > 0) {
        --spawn_timer;
    } else {
        // Find inactive slot
        for (i = 0; i < MAX_OBSTACLES; ++i) {
            if (!obstacles[i].active) {
                unsigned char r = rand8() % 3;
                obstacles[i].active = 1;
                obstacles[i].x = 240;
                obstacles[i].type = r;
                obstacles[i].width = 12;

                if (r == OBS_DRONE) {
                    obstacles[i].y = 136; // Flying height: must duck!
                    obstacles[i].height = 10;
                } else {
                    obstacles[i].y = 154; // Ground height: must jump!
                    obstacles[i].height = 14;
                }

                // Random gap to next spawn
                spawn_timer = 50 + (rand8() % 40) - (speed_px * 4);
                if (spawn_timer < 35) spawn_timer = 35;
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

// --- COLLISION DETECTION ---
static unsigned char check_player_collision(void) {
    unsigned char i;
    unsigned char py = (player_y_sub >> 8);
    struct Box pbox;

    pbox.x = GEEKO_X + 4;  // sprite spans x 0..23; box leaves the rump and snout tip out
    pbox.width = 16;

    if (is_ducking && !is_jumping) {
        pbox.y = py + 10;
        pbox.height = 10; // Low crouch profile
    } else {
        pbox.y = py + 2;
        pbox.height = 20; // Full standing profile
    }

    for (i = 0; i < MAX_OBSTACLES; ++i) {
        if (obstacles[i].active) {
            struct Box obox;
            obox.x = obstacles[i].x + 2;
            obox.y = obstacles[i].y + 2;
            obox.width = obstacles[i].width;
            obox.height = obstacles[i].height;

            if (check_collision(&pbox, &obox)) {
                return 1; // Collision detected
            }
        }
    }
    return 0;
}

// --- SPRITE RENDERING ---
static void set_sprite_zero(void) {
    // Sprite 0 at Y = 20, X = 240, hits tile 0x80 at NTADR_A(30, 2)
    oam_spr(240, 20, 0x20, 3);
}

static void render_all_sprites(void) {
    unsigned char i;
    unsigned char py = (player_y_sub >> 8);

    oam_clear();
    set_sprite_zero();
    oam_set(4); // Start user sprites at OAM index 4

    // 1. Draw Obstacles first: the PPU drops sprites past 8 per scanline, lowest OAM
    //    priority first. Geeko is 3 tiles wide, so obstacles (2 x 2 tiles) + Geeko +
    //    fire trail can exceed 8; make the trail cosmetic loss, not an obstacle.
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

    // 2. Draw Geeko McFly
    if (game_state == STATE_GAMEOVER) {
        // Crash burst over Geeko
        oam_meta_spr(GEEKO_X + 12, py + 2, SprCrash);
        oam_meta_spr(GEEKO_X, py, GeekoDuck);
    } else if (is_ducking && !is_jumping) {
        oam_meta_spr(GEEKO_X, py, GeekoDuck);
    } else {
        // Add subtle hoverboard floating bob
        unsigned char bob = (is_jumping) ? 0 : ((bob_timer >> 3) & 1);
        oam_meta_spr(GEEKO_X, py + bob, GeekoRide);
    }

    // 3. Draw 88 MPH Fire Trail if at warp speed
    if (mph >= 88) {
        ++fire_trail_frame;
        oam_meta_spr(GEEKO_X - 12, py + 14, SprFireTrail);
        if (fire_trail_frame & 2) {
            oam_meta_spr(GEEKO_X - 20, py + 15, SprFireTrail);
        }
    }
}

// --- MAIN LOOP ---
int main(void) {
    sfx_init();
    ppu_off();

    pal_bg(bg_palette);
    pal_spr(spr_palette);

    bank_spr(1); // Use CHR pattern table 1 for sprites
    bank_bg(0);  // Use CHR pattern table 0 for background & font

    set_vram_buffer();

    draw_title_screen();

    while (1) {
        ppu_wait_nmi();

        // Sprite 0 hit screen split:
        // Top HUD is stationary (scroll = 0).
        // Ground below scrolls with scroll_x.
        if (game_state == STATE_PLAYING) {
            split(scroll_x);
            set_scroll_x(0);
            set_scroll_y(0);
        } else {
            set_scroll_x(0);
            set_scroll_y(0);
        }

        pad1 = pad_poll(0);
        pad1_new = get_pad_new(0);

        // --- STATE: TITLE SCREEN ---
        if (game_state == STATE_TITLE) {
            if (pad1_new & (PAD_START | PAD_A)) {
                seed_rng();
                init_game_session();
                game_state = STATE_PLAYING;
            }
        }
        // --- STATE: PLAYING ---
        else if (game_state == STATE_PLAYING) {
            ++bob_timer;

            // Player Jump Input
            if ((pad1_new & (PAD_A | PAD_UP)) && !is_jumping) {
                is_jumping = 1;
                player_vel_y = JUMP_IMPULSE;
                sfx_jump();
            }

            // Player Duck Input
            if (pad1 & PAD_DOWN) {
                if (!is_ducking && !is_jumping) {
                    sfx_duck();
                }
                is_ducking = 1;
            } else {
                is_ducking = 0;
            }

            // Jump Physics
            if (is_jumping) {
                player_vel_y += GRAVITY;
                player_y_sub += player_vel_y;

                // Land on ground
                if (player_y_sub >= (GROUND_Y << 8)) {
                    player_y_sub = (GROUND_Y << 8);
                    player_vel_y = 0;
                    is_jumping = 0;
                }
            }

            // Progression: Speed & Score
            scroll_x += speed_px;

            ++score_sub;
            if (score_sub >= 4) {
                score_sub = 0;
                ++score;

                // Milestone ping every 100 points
                if (score % 100 == 0) {
                    sfx_beep();
                }

                // Speed calculation: 20 MPH -> 88 MPH
                mph = 20 + (score / 40);
                if (mph > 88) {
                    mph = 88;
                }

                // Warp trigger at 88 MPH
                if (mph == 88 && warp_flash_timer == 0) {
                    warp_flash_timer = 30;
                    sfx_warp();
                }

                // Pixel scroll speed
                speed_px = 2 + (mph / 25);
            }

            update_obstacles();

            // Collision Check
            if (check_player_collision()) {
                sfx_crash();
                game_state = STATE_GAMEOVER;
                if (score > hi_score) {
                    hi_score = score;
                }
                // Print Crash notice in VRAM buffer
                multi_vram_buffer_horz(" CRASH! ", 8, NTADR_A(12, 10));
                multi_vram_buffer_horz("PRESS START", 11, NTADR_A(10, 12));
            }

            update_hud_vram();
        }
        // --- STATE: GAME OVER ---
        else if (game_state == STATE_GAMEOVER) {
            if (pad1_new & (PAD_START | PAD_A)) {
                init_game_session();
                game_state = STATE_PLAYING;
            }
        }

        render_all_sprites();
    }
}
