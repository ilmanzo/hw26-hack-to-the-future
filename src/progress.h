#ifndef PROGRESS_H
#define PROGRESS_H

// Run progression: world scroll, score, speed in MPH. Written only by progress.c.
// Speed rises from 20 to 88 MPH as the score grows; 88 MPH is the time-warp trigger.

#define MPH_START   20
#define MPH_WARP    88

extern unsigned int  scroll_x;    // world scroll in pixels (wraps at 65536, nametables wrap at 512)
extern unsigned char speed_px;    // pixels scrolled per frame
extern unsigned char mph;
extern unsigned int  score;
extern unsigned int  hi_score;    // survives restarts, lost on power-off

void progress_reset(void);

// One frame: scroll, then every 4th frame score/speed (and the 88 MPH warp trigger).
void progress_update(void);

// Call once when the run ends.
void progress_game_over(void);

#endif // PROGRESS_H
