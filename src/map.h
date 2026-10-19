#ifndef MAP_H
#define MAP_H

// The time map shown before each level (plan.md 8a.8): the eras on a line, finished ones green,
// Geeko blinking over the one that comes next. State is private to map.c.

// Draw the map for the era about to start (0-3, 4 = the boss). Turns rendering off and on again.
void map_show(unsigned char epoch);

// Move the selection to era epoch (0-4) on a map that is on screen: the nodes before it turn green,
// the ones from it on blue, and the caption changes. Through the VRAM buffer, rendering stays on.
void map_select(unsigned char epoch);

// Geeko over the era's node, blinking; timer = frames since the map appeared.
void map_draw_geeko(unsigned char epoch, unsigned char timer);

#endif // MAP_H
