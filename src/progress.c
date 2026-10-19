#include "progress.h"
#include "sfx.h"
#include "tuning.h"

unsigned int  scroll_x = 0;
unsigned char speed_px = 2;
unsigned char mph = MPH_START;
unsigned int  score = 0;
unsigned int  hi_score = 0;
unsigned int  dodges = 0;
unsigned char mult = 1;
unsigned int  time_left = GAME_TIME_S * 60u;
unsigned char invuln = 0;
unsigned char epoch = 0;
unsigned char year_idx = 0;
unsigned int  race_frames = 0;

static unsigned char mph_sub = 0;     // frames since the last +1 MPH
static unsigned char score_sub = 0;   // frames since the last speed points
static unsigned char streak = 0;      // obstacles passed in a row

// Invulnerability when nothing hit Geeko: none, or with the cheat a blink that never ends (below the
// crash pose, so Geeko does not show the burst all the time).
#if DEV_INVULNERABLE
#define INVULN_IDLE (INVULN_FRAMES - CRASH_POSE_FRAMES)
#else
#define INVULN_IDLE 0
#endif

static void speed_update(void) {
    speed_px = 2 + mph / 25;
}

void progress_reset(void) {
    score = 0;
    dodges = 0;
    time_left = GAME_TIME_S * 60u;
    epoch = 0;
    year_idx = 0;
    race_start();
}

void race_start(void) {
    scroll_x = 0;
    mph = MPH_START;
    mph_sub = 0;
    score_sub = 0;
    streak = 0;
    mult = 1;
    invuln = INVULN_IDLE;
    race_frames = 0;
    speed_update();
}

void progress_update(void) {
    scroll_x += speed_px;
    ++race_frames;

    if (mph < MPH_WARP && ++mph_sub >= RACE_FRAMES_PER_MPH) {
        mph_sub = 0;
        ++mph;
        speed_update();
    }

    if (++score_sub >= SCORE_TICK_FRAMES) {
        score_sub = 0;
        score_add(mph / SCORE_SPEED_DIV);
    }
}

void progress_hit(void) {
    invuln = INVULN_FRAMES;
    clock_sub(HIT_TIME_PENALTY_S);
}

void progress_crash(void) {
    mph = mph > MPH_START + CRASH_MPH_LOSS ? mph - CRASH_MPH_LOSS : MPH_START;
    mph_sub = 0;
    speed_update();
    streak = 0;
    mult = 1;
    progress_hit();
}

void progress_room_start(void) {
    invuln = INVULN_IDLE;
}

void progress_dodge(void) {
    unsigned char m;

    ++dodges;
    ++streak;
    m = 1 + streak / DODGE_STREAK_STEP;
    if (m > MULT_MAX) m = MULT_MAX;
    if (m > mult) sfx_beep();
    mult = m;
    score_add((unsigned int)DODGE_POINTS * mult);
}

void score_add(unsigned int points) {
    score = (unsigned int)(score + points) < score ? 0xffff : score + points;
}

void clock_add(unsigned char seconds) {
    time_left += seconds * 60u;
    if (time_left > TIME_MAX_S * 60u) time_left = TIME_MAX_S * 60u;
}

void clock_sub(unsigned char seconds) {
    unsigned int d = seconds * 60u;
    time_left = time_left > d ? time_left - d : 0;
}

unsigned char clock_tick(void) {
    if (invuln) --invuln;
    if (!invuln) invuln = INVULN_IDLE;
    if (time_left) --time_left;
    return time_left != 0;
}

void progress_game_over(void) {
    if (score > hi_score) {
        hi_score = score;
    }
}

const char *rank_name(void) {
    if (score >= RANK_4_SCORE) return "ROOT         ";
    if (score >= RANK_3_SCORE) return "SYSADMIN     ";
    if (score >= RANK_2_SCORE) return "SCRIPT KIDDIE";
    return "NEWBIE       ";
}

const char *year_text(void) {
    static const char years[][4] = { {'1','9','8','5'}, {'1','9','9','0'}, {'2','0','0','0'},
                                     {'2','0','1','0'}, {'2','0','2','0'} };
    return years[year_idx];
}

unsigned int year_value(unsigned char idx) {
    static const unsigned int years[] = { 1985, 1990, 2000, 2010, 2020 };
    return years[idx];
}
