#ifndef RENDER_H
#define RENDER_H

// Builds the OAM for one frame. This is the only place that decides draw order.
// crashed: game-over pose.
void render_all_sprites(unsigned char crashed);

#endif // RENDER_H
