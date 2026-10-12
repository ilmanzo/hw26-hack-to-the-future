#include "progress.h"
#include "sfx.h"

unsigned int  scroll_x = 0;
unsigned char speed_px = 2;
unsigned char mph = MPH_START;
unsigned int  score = 0;
unsigned int  hi_score = 0;

static unsigned char score_sub = 0;         // frames since the last score tick
static unsigned char warp_flash_timer = 0;  // set at 88 MPH; the flash itself is not implemented yet

void progress_reset(void) {
    scroll_x = 0;
    speed_px = 2;
    mph = MPH_START;

    score = 0;
    score_sub = 0;
    warp_flash_timer = 0;
}

void progress_update(void) {
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
        mph = MPH_START + (score / 40);
        if (mph > MPH_WARP) {
            mph = MPH_WARP;
        }

        // Warp trigger at 88 MPH
        if (mph == MPH_WARP && warp_flash_timer == 0) {
            warp_flash_timer = 30;
            sfx_warp();
        }

        // Pixel scroll speed
        speed_px = 2 + (mph / 25);
    }
}

void progress_game_over(void) {
    if (score > hi_score) {
        hi_score = score;
    }
}
