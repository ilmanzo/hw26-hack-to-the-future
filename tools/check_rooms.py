#!/usr/bin/env python3
"""Check the room maps of plan.md section 8a/8b: is every terminal and the gate reachable?

    python3 tools/check_rooms.py        # one line per room, exit 1 if any room fails
    python3 tools/check_rooms.py -v     # also print each map as ASCII

Rooms are rectangles (row0, row1, col0, col1, inclusive, tile units) plus object lists: the input
format of tools/build_rooms.py, which turns the playable rooms into assets/rooms.h for the game.
This script is a model of the jump, not the emulator: a recorded-input playthrough is
still the proof. The jump numbers come from src/geeko.h (same fixed-point order), with one tile
(8 px) of margin everywhere.
"""
import os
import re
import sys

W, H = 32, 30
FLOOR = 26          # top row of the floor
MARGIN = 8          # px of safety on every limit
DROP_GAP = 6        # tiles: widest gap for a drop onto a lower surface

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
_geeko_h = open(os.path.join(ROOT, "src", "geeko.h")).read()


def _const(pattern, name):
    m = re.search(pattern, _geeko_h)
    if not m:
        sys.exit(f"check_rooms.py: cannot find {name} in src/geeko.h (renamed? update the pattern)")
    return int(m.group(1), 0)


GRAVITY = _const(r"#define\s+GEEKO_GRAVITY\s+(0x[0-9a-fA-F]+|\d+)", "GEEKO_GRAVITY")
IMPULSE = _const(r"#define\s+GEEKO_JUMP\s+(0x[0-9a-fA-F]+|\d+)", "GEEKO_JUMP")


def frames_to_land(rise_px):
    """Frames from take-off until the arc comes back down through height rise_px (None: never gets there).
    Also returns the apex in px. 8.8 fixed point: vel += gravity; y += vel, like player.c."""
    y, v, t, apex = 0, -IMPULSE, 0, 0
    while t < 300:
        v += GRAVITY
        y += v
        t += 1
        apex = max(apex, -y)
        if v > 0 and -y <= rise_px * 256:
            return t, apex / 256
    return None, apex / 256


APEX = frames_to_land(0)[1]
MAX_RISE = int((APEX - MARGIN) // 8)    # tiles


def max_gap(rise):
    """Widest edge-to-edge gap (tiles) for a jump up by `rise` tiles, at 1 px/frame (a partial-speed jump)."""
    if rise > MAX_RISE:
        return -1
    frames, _ = frames_to_land(rise * 8)
    return (frames - MARGIN) // 8


class Room:
    def __init__(self, name, rects, terminals, gate, start, foes=()):
        self.name, self.terminals, self.gate, self.start, self.foes = name, terminals, gate, start, foes
        self.rects_list = list(rects)
        self.solid = [[False] * W for _ in range(H)]
        for r0, r1, c0, c1 in rects:
            for r in range(r0, r1 + 1):
                for c in range(c0, c1 + 1):
                    self.solid[r][c] = True

    def free(self, r, c):
        return 0 <= r < H and 0 <= c < W and not self.solid[r][c]

    def surfaces(self):
        """Runs (row, col0, col1) of solid cells with 2 free rows above (Geeko is 2 tiles tall)."""
        runs = []
        for r in range(3, H):
            c = 0
            while c < W:
                if self.solid[r][c] and self.free(r - 1, c) and self.free(r - 2, c):
                    c0 = c
                    while c + 1 < W and self.solid[r][c + 1] and self.free(r - 1, c + 1) and self.free(r - 2, c + 1):
                        c += 1
                    runs.append((r, c0, c))
                c += 1
        return runs

    def blocked(self, ra, rb, lo, hi):
        """A solid cell in the gap columns reaching up to the higher of the two surfaces."""
        for c in range(lo, hi + 1):
            for r in range(3, max(ra, rb)):
                if self.solid[r][c] and r >= min(ra, rb):
                    return True
        return False

    def edges(self, runs):
        adj = {i: set() for i in range(len(runs))}
        for i, (ra, a0, a1) in enumerate(runs):
            for j, (rb, b0, b1) in enumerate(runs):
                if i == j:
                    continue
                overlap = not (b1 < a0 or b0 > a1)
                if b1 < a0:
                    gap, lo, hi = a0 - b1 - 1, b1 + 1, a0 - 1
                elif b0 > a1:
                    gap, lo, hi = b0 - a1 - 1, a1 + 1, b0 - 1
                else:
                    gap, lo, hi = 0, 0, -1
                rise = ra - rb                       # > 0: B is higher
                if rise > 0:
                    if overlap:                      # B above A: jump from a column of A beside B's end
                        if (a0 < b0 or a1 > b1) and max_gap(rise) >= 0:
                            adj[i].add(j)
                    elif 0 <= gap <= max_gap(rise) and not self.blocked(ra, rb, lo, hi):
                        adj[i].add(j)
                elif rise == 0:
                    if not overlap and gap <= max_gap(0) and not self.blocked(ra, rb, lo, hi):
                        adj[i].add(j)
                else:                                # drop
                    if overlap or (gap <= DROP_GAP and not self.blocked(ra, rb, lo, hi)):
                        adj[i].add(j)
        return adj

    def problems(self):
        runs = self.surfaces()
        adj = self.edges(runs)

        def run_of(r, c):
            for i, (rr, a, b) in enumerate(runs):
                if rr == r and a <= c <= b:
                    return i

        src = run_of(*self.start)
        seen, todo = {src}, [src]
        while todo:
            for v in adj[todo.pop()]:
                if v not in seen:
                    seen.add(v)
                    todo.append(v)
        bad = []
        for k, (r, c) in enumerate(self.terminals):
            i = run_of(r, c)
            if i is None or i not in seen:
                bad.append(f"terminal {k + 1} at row {r} col {c}")
        if self.gate:
            i = run_of(self.gate[0], self.gate[1])
            if i is None or i not in seen:
                bad.append("gate")
        return bad

    def ascii(self):
        g = [["#" if self.solid[r][c] else "." for c in range(W)] for r in range(H)]
        for r, c in self.terminals:
            for dr in (1, 2):
                for dc in (0, 1):
                    g[r - dr][c + dc] = "T"
        if self.gate:
            r, c, w = self.gate
            for dr in range(1, 4):
                for dc in range(w):
                    g[r - dr][c + dc] = "G"
        r, c = self.start
        g[r - 1][c] = "S"
        for foe in self.foes:
            g[foe[1]][foe[2]] = foe[0]
        first = next(r for r in range(3, H) if set(g[r]) != {"."})
        out = ["    " + "".join(str(c % 10) for c in range(W))]
        out += [f"{r:2d}  " + "".join(g[r]) for r in range(max(3, first - 1), H)]
        return "\n".join(out)


FL = (FLOOR, 29, 0, 31)
# foes: (letter, row, col, ...) = where a foe starts; behaviour and range are in plan.md 8a.5. A drone
# 'd' is (letter, row, first col, last col): it patrols that span at 0.5 px/frame.
ROOMS = [
    Room("1990 UNIX Labs", [FL, (22, 22, 3, 9), (18, 18, 12, 21), (22, 22, 24, 29)],
         terminals=[(18, 16), (22, 26)], gate=(26, 0, 2), start=(26, 10),
         foes=[("d", 23, 14, 22)]),
    Room("2000 Dot-com farm", [FL, (23, 25, 9, 10), (23, 25, 24, 25), (22, 22, 1, 6), (18, 18, 8, 15),
                               (22, 22, 17, 22), (18, 18, 26, 30)],
         terminals=[(22, 3), (18, 11), (18, 28)], gate=(26, 29, 2), start=(26, 7),
         foes=[("b", 25, 12), ("p", 20, 7), ("c", 4, 13), ("c", 4, 19)]),
    Room("2010 Cloud cleanroom", [FL, (22, 22, 24, 29), (18, 18, 17, 22), (14, 14, 9, 14)],
         terminals=[(22, 26), (18, 19), (14, 11)], gate=(26, 0, 2), start=(26, 4),
         foes=[("l", 25, 12), ("s", 4, 12), ("k", 25, 6)]),
    # the platform (18, 10-17) MOVES (4 tiles wide, sliding over that span); modelled as fixed
    Room("2020 AI edge", [FL, (22, 22, 2, 7), (22, 22, 24, 29), (18, 18, 10, 17), (14, 14, 14, 19)],
         terminals=[(22, 4), (22, 26), (14, 16), (26, 15)], gate=(26, 30, 2), start=(26, 0),
         foes=[("h", 22, 20), ("o", 20, 7), ("o", 20, 22)]),
    Room("Boss: THE MAINFRAME", [FL, (22, 22, 1, 7), (22, 22, 24, 30), (18, 18, 8, 13), (18, 18, 18, 23),
                                 (14, 14, 3, 8), (14, 14, 23, 28)],
         terminals=[(22, 3), (22, 27), (18, 10), (18, 20), (14, 5), (14, 25)], gate=None, start=(26, 15)),
]


def selftest():
    """The checker must say yes to possible rooms and no to impossible ones (start: floor col 2)."""
    def problems(*rects, terminal):
        return Room("t", [FL, *rects], terminals=[terminal], gate=None, start=(26, 2)).problems()
    assert not problems((22, 22, 4, 8), terminal=(22, 6)), "rise 4 from the floor, beside the platform"
    assert not problems((22, 22, 4, 8), (18, 18, 10, 14), terminal=(18, 12)), "rise 4, gap 1"
    assert problems((22, 22, 4, 8), (18, 18, 14, 18), terminal=(18, 16)), "rise 4, gap 5 is too wide"
    assert problems((20, 20, 10, 14), terminal=(20, 12)), "rise 6 from the floor is too high"


if __name__ == "__main__":
    selftest()
    failed = False
    for room in ROOMS:
        bad = room.problems()
        failed = failed or bool(bad)
        print(f"{room.name}: " + ("all reachable" if not bad else "UNREACHABLE " + ", ".join(bad)))
        if "-v" in sys.argv:
            print(room.ascii() + "\n")
    sys.exit(1 if failed else 0)
