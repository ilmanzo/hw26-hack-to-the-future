#ifndef PLAYER_H
#define PLAYER_H

#include "game.h"

// Geeko: input, jump physics, hitbox and drawing. State is private to player.c.

void player_reset(void);

// One frame of input + physics. pad = buttons held, pad_new = buttons pressed this frame.
void player_update(unsigned char pad, unsigned char pad_new);

void player_get_box(struct Box *box);

// crashed: draw the game-over pose and the crash burst.
void player_draw(unsigned char crashed);

// Screen Y of the sprite's top edge (the hoverboard bob is not included).
unsigned char player_y(void);

#endif // PLAYER_H
