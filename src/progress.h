#ifndef PROGRESS_H
#define PROGRESS_H

// Run progression: world scroll, speed in MPH, score, the clock. Written only by progress.c.
// A race lasts until 88 MPH (the time-warp trigger); a crash only slows Geeko and costs time, the
// only game over is the clock reaching 0. Balance numbers live in tuning.h.

#define MPH_START   20
#define MPH_WARP    88

extern unsigned int  scroll_x;    // world scroll in pixels (wraps at 65536, nametables wrap at 512)
extern unsigned char speed_px;    // pixels scrolled per frame
extern unsigned char mph;
extern unsigned int  score;
extern unsigned int  hi_score;    // survives restarts, lost on power-off
extern unsigned int  dodges;      // obstacles passed untouched this game
extern unsigned char mult;        // score multiplier, 1..MULT_MAX
extern unsigned int  time_left;   // the clock, in frames (16 bits: 65535 frames = 1092 s at most)
extern unsigned char invuln;      // frames of invulnerability left after a hit
extern unsigned char year_idx;    // index into the year names, see year_text()
extern unsigned int  race_frames; // frames of the current race (for the result card)

void progress_reset(void);        // new game: score 0, full clock, first race
void race_start(void);            // new race: 20 MPH, scroll 0; score and clock carry over

// One race frame: scroll, speed, score ticks, invulnerability countdown.
void progress_update(void);

void progress_hit(void);          // any hit: lose time, start the invulnerability
void progress_crash(void);        // race crash: a hit, plus lose speed, multiplier back to 1
void progress_room_start(void);   // entering a room: no invulnerability left over
void progress_dodge(void);        // an obstacle passed untouched: streak, multiplier, points

void score_add(unsigned int points);   // saturates at 65535
void clock_add(unsigned char seconds); // saturates at TIME_MAX_S
void clock_sub(unsigned char seconds); // saturates at 0
unsigned char clock_tick(void);        // one frame (race or room): the clock and the invulnerability count down;
                                       // returns 0 once the clock has run out

// Call once when the run ends: updates the hi-score.
void progress_game_over(void);

const char *rank_name(void);      // 13 characters, padded, from the score
const char *year_text(void);      // 4 characters, from year_idx

#endif // PROGRESS_H
