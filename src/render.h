#ifndef RENDER_H
#define RENDER_H

// Builds the OAM for one frame. This is the only place that decides draw order.
// crashed: crash pose and burst (first frames after a hit).
void render_all_sprites(unsigned char crashed);

// A room: foes, then Geeko (hacking: hack pose).
void render_room_sprites(unsigned char hacking);

// Nothing but sprite 0 (result card).
void render_clear(void);

#endif // RENDER_H
