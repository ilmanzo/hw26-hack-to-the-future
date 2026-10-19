#ifndef OBSTACLES_H
#define OBSTACLES_H

#include "game.h"

// Bugs, tumbleweeds (ground: jump) and drones (flying: duck) that scroll in from the right.

void obstacles_reset(void);

// One frame: spawn countdown and spawning, then move every active obstacle left.
void obstacles_update(void);

// 1 if any active obstacle overlaps the box (that obstacle then gives no dodge points).
unsigned char obstacles_hit(const struct Box *box);

void obstacles_draw(void);

#endif // OBSTACLES_H
