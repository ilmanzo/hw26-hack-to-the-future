#ifndef TUNING_H
#define TUNING_H

// Game-balance numbers, all in one place. Plain numeric #defines (no expressions, no casts):
// change a value, `make`, play. tests/*.lua read this file, so they follow your values.
// Units: _S = seconds, _FRAMES = frames (60 per second, NTSC).
// Used by progress.c, hero.c, room.c, main.c and read by the tests; see plan.md 8a.6 and 8a.7.

// The clock: one for the whole game.
#define GAME_TIME_S          600    // 10 minutes: 36,000 frames, fits the 16-bit frame counter
#define TIME_MAX_S           600    // bonuses never push the clock above this
#define HIT_TIME_PENALTY_S   5      // lost on a race crash and on contact with a foe or agent
#define HACK_TIME_BONUS_S    10     // gained for every hacked terminal

// The clock is a 16-bit count of frames: 65535 frames = 1092 s is the most it can hold.
#if GAME_TIME_S > 1092 || TIME_MAX_S > 1092
#error "GAME_TIME_S and TIME_MAX_S must be at most 1092 (16-bit frame counter)"
#endif

// Race.
#define RACE_FRAMES_PER_MPH  53     // +1 MPH per this many frames: 20 -> 88 MPH in about 60 s
#define CRASH_MPH_LOSS       10     // speed lost on a crash (never below 20)
#define INVULN_FRAMES        60     // no second hit for this long after a hit (race and rooms)

// Score (plan.md 8a.7). Tuned so that a flawless run is worth about 26,000 points (tests/smoke.lua checks the model).
#define SCORE_TICK_FRAMES    4      // race: every this many frames the speed adds points
#define SCORE_SPEED_DIV      20     // ... mph / this (20 MPH gives 1 point per tick, 88 MPH gives 4)
#define DODGE_POINTS         10     // per obstacle passed untouched, times the multiplier
#define DODGE_STREAK_STEP    3      // the multiplier grows by 1 every this many dodges in a row
#define MULT_MAX             5      // a hit puts the multiplier back to 1
#define TERMINAL_POINTS      100    // per hacked terminal
#define CLEAN_ROOM_POINTS    250    // for a room finished without being hit
#define FINISH_POINTS_PER_S  10     // for winning: seconds left on the clock times this

// Ranks, by final score (also shown on the game-over screen).
#define RANK_2_SCORE         5000   // SCRIPT KIDDIE
#define RANK_3_SCORE         13000  // SYSADMIN
#define RANK_4_SCORE         21000  // ROOT (below RANK_2_SCORE: NEWBIE)

// Development: on the title screen, SELECT starts room 1 directly (set to 0 for a release build).
#define DEV_SELECT_ROOM      1

// Hacking: hold B this long next to a terminal.
#define HACK_FRAMES_1990     60
#define HACK_FRAMES_2000     75
#define HACK_FRAMES_2010     90
#define HACK_FRAMES_2020     105
#define HACK_FRAMES_BOSS     120

#endif // TUNING_H
