#ifndef TUNING_H
#define TUNING_H

// Game-balance numbers, all in one place. Plain numeric #defines (no expressions, no casts):
// change a value, `make`, play. tests/*.lua read this file, so they follow your values.
// Units: _S = seconds, _FRAMES = frames (60 per second, NTSC).
// Nothing includes this file yet: the clock and the slowdown arrive in plan.md steps 2.3 and 2.4.

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

// Hacking: hold B this long next to a terminal.
#define HACK_FRAMES_1990     60
#define HACK_FRAMES_2000     75
#define HACK_FRAMES_2010     90
#define HACK_FRAMES_2020     105
#define HACK_FRAMES_BOSS     120

#endif // TUNING_H
