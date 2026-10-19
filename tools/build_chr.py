#!/usr/bin/env python3
"""
build_chr.py
Generates an 8KB NES 2bpp CHR-ROM file for "Hack to the Future: Geeko McFly".
Pattern Table 0 ($0000-$0FFF): Background, Font & HUD
Pattern Table 1 ($1000-$1FFF): Sprites (Geeko, Hoverboard, Obstacles, FX)
"""

import sys

CHR_SIZE = 8192
TABLE_SIZE = 4096
TILE_SIZE = 16

def encode_tile(lines):
    """
    Encodes an 8x8 tile from 8 strings of 8 characters each.
    ' ' or '.' = Color 0 (transparent / background)
    '1' or 'x' = Color 1 (e.g. Geeko Green / Road Grey)
    '2' or 'o' = Color 2 (e.g. McFly Orange / Yellow accent)
    '3' or '*' = Color 3 (e.g. White / Highlight)
    """
    plane0 = bytearray(8)
    plane1 = bytearray(8)
    
    for y, line in enumerate(lines[:8]):
        p0 = 0
        p1 = 0
        for x, ch in enumerate(line[:8].ljust(8)):
            c = 0
            if ch in ('1', 'x', 'X', 'g', 'G'):
                c = 1
            elif ch in ('2', 'o', 'O', 'v', 'V'):
                c = 2
            elif ch in ('3', '*', '#', 'w', 'W'):
                c = 3
            bit = 7 - x
            if c & 1:
                p0 |= (1 << bit)
            if c & 2:
                p1 |= (1 << bit)
        plane0[y] = p0
        plane1[y] = p1
        
    return bytes(plane0 + plane1)

def put_sprite(tiles, first, art):
    """Slice an ASCII picture (width/height multiples of 8) into tiles, row-major, from index `first`."""
    cols = len(art[0]) // 8
    for ty in range(len(art) // 8):
        for tx in range(cols):
            tiles[first + ty * cols + tx] = encode_tile([row[tx * 8:tx * 8 + 8] for row in art[ty * 8:ty * 8 + 8]])

def blank_tile():
    return bytes(TILE_SIZE)

def solid_tile(color):
    lines = [str(color) * 8 for _ in range(8)]
    return encode_tile(lines)

# Simple 5x7 / 8x8 font bitmap for ASCII 0x20..0x5F
FONT_RAW = {
    ' ': ["        "] * 8,
    '!': ["   *    ","   *    ","   *    ","   *    ","        ","   *    ","        ","        "],
    '"': ["  * *   ","  * *   ","        ","        ","        ","        ","        ","        "],
    '#': [" * *    ","*****   "," * *    ","*****   "," * *    ","        ","        ","        "],
    "'": ["   *    ","   *    ","  *     ","        ","        ","        ","        ","        "],
    '%': ["**   *  ","**  *   ","   *    ","  *   **"," *    **","        ","        ","        "],
    '(': ["   *    ","  *     ","  *     ","  *     ","   *    ","        ","        ","        "],
    ')': ["  *     ","   *    ","   *    ","   *    ","  *     ","        ","        ","        "],
    '+': ["        ","   *    "," *****  ","   *    ","        ","        ","        ","        "],
    ',': ["        ","        ","        ","        ","        ","   *    ","   *    ","  *     "],
    '-': ["        ","        "," *****  ","        ","        ","        ","        ","        "],
    '.': ["        ","        ","        ","        ","        ","  **    ","  **    ","        "],
    '/': ["     *  ","    *   ","   *    ","  *     "," *      ","        ","        ","        "],
    ':': ["        ","  **    ","  **    ","        ","  **    ","  **    ","        ","        "],
    '0': [" ****   ","*    *  ","*   **  ","* *  *  ","**   *  ","*    *  "," ****   ","        "],
    '1': ["   *    ","  **    ","   *    ","   *    ","   *    ","   *    ","  ***   ","        "],
    '2': [" ****   ","*    *  ","     *  ","   **   "," **     ","*       ","******  ","        "],
    '3': [" ****   ","*    *  ","     *  ","  ***   ","     *  ","*    *  "," ****   ","        "],
    '4': ["*   *   ","*   *   ","*   *   ","******  ","    *   ","    *   ","    *   ","        "],
    '5': ["******  ","*       ","*****   ","     *  ","     *  ","*    *  "," ****   ","        "],
    '6': [" ****   ","*    *  ","*       ","*****   ","*    *  ","*    *  "," ****   ","        "],
    '7': ["******  ","     *  ","    *   ","   *    ","  *     ","  *     ","  *     ","        "],
    '8': [" ****   ","*    *  ","*    *  "," ****   ","*    *  ","*    *  "," ****   ","        "],
    '9': [" ****   ","*    *  ","*    *  "," *****  ","     *  ","*    *  "," ****   ","        "],
    'A': ["  **    "," *  *   ","*    *  ","******  ","*    *  ","*    *  ","*    *  ","        "],
    'B': ["*****   ","*    *  ","*    *  ","*****   ","*    *  ","*    *  ","*****   ","        "],
    'C': [" ****   ","*    *  ","*       ","*       ","*       ","*    *  "," ****   ","        "],
    'D': ["*****   ","*    *  ","*    *  ","*    *  ","*    *  ","*    *  ","*****   ","        "],
    'E': ["******  ","*       ","*       ","*****   ","*       ","*       ","******  ","        "],
    'F': ["******  ","*       ","*       ","*****   ","*       ","*       ","*       ","        "],
    'G': [" ****   ","*    *  ","*       ","*  ***  ","*    *  ","*    *  "," ****   ","        "],
    'H': ["*    *  ","*    *  ","*    *  ","******  ","*    *  ","*    *  ","*    *  ","        "],
    'I': [" ****   ","  **    ","  **    ","  **    ","  **    ","  **    "," ****   ","        "],
    'J': ["    **  ","    **  ","    **  ","    **  ","*   **  ","*   **  "," ****   ","        "],
    'K': ["*    *  ","*   *   ","*  *    ","***     ","*  *    ","*   *   ","*    *  ","        "],
    'L': ["*       ","*       ","*       ","*       ","*       ","*       ","******  ","        "],
    'M': ["*    *  ","**  **  ","* ** *  ","*    *  ","*    *  ","*    *  ","*    *  ","        "],
    'N': ["*    *  ","**   *  ","* *  *  ","*  * *  ","*   **  ","*    *  ","*    *  ","        "],
    'O': [" ****   ","*    *  ","*    *  ","*    *  ","*    *  ","*    *  "," ****   ","        "],
    'P': ["*****   ","*    *  ","*    *  ","*****   ","*       ","*       ","*       ","        "],
    'Q': [" ****   ","*    *  ","*    *  ","*    *  ","*  * *  ","*   **  "," **** * ","        "],
    'R': ["*****   ","*    *  ","*    *  ","*****   ","*  *    ","*   *   ","*    *  ","        "],
    'S': [" ****   ","*    *  ","*       "," ****   ","     *  ","*    *  "," ****   ","        "],
    'T': ["******  ","  **    ","  **    ","  **    ","  **    ","  **    ","  **    ","        "],
    'U': ["*    *  ","*    *  ","*    *  ","*    *  ","*    *  ","*    *  "," ****   ","        "],
    'V': ["*    *  ","*    *  ","*    *  ","*    *  "," *  *   ","  **    ","  **    ","        "],
    'W': ["*    *  ","*    *  ","*    *  ","* ** *  ","**  **  ","*    *  ","*    *  ","        "],
    'X': ["*    *  "," *  *   ","  **    ","  **    "," *  *   ","*    *  ","*    *  ","        "],
    'Y': ["*    *  "," *  *   ","  **    ","  **    ","  **    ","  **    ","  **    ","        "],
    'Z': ["******  ","     *  ","    *   ","   *    ","  *     "," *      ","******  ","        "],
}

def make_bg_table():
    tiles = [blank_tile()] * 256
    
    # 1. Populate ASCII font into tiles 0x20 .. 0x5F
    for char, grid in FONT_RAW.items():
        idx = ord(char)
        if idx < 256:
            tiles[idx] = encode_tile(grid)
            
    # 2. Road / Ground Tiles (Tiles 0x80 - 0x8F)
    # 0x80: Horizon line / cyber grid top
    tiles[0x80] = encode_tile([
        "********",
        "        ",
        "11111111",
        "        ",
        "        ",
        "11111111",
        "        ",
        "        "
    ])
    
    # 0x81: Road surface with circuit traces
    tiles[0x81] = encode_tile([
        "11111111",
        "1  1   1",
        "1 11 1 1",
        "1    1 1",
        "111111 1",
        "1      1",
        "1 111111",
        "11111111"
    ])
    
    # 0x82: Road center dashed line
    tiles[0x82] = encode_tile([
        "22222222",
        "22222222",
        "        ",
        "        ",
        "        ",
        "        ",
        "        ",
        "        "
    ])

    # 0x83: Deep bedrock / solid ground
    tiles[0x83] = encode_tile([
        "11111111",
        "11111111",
        "11111111",
        "11111111",
        "11111111",
        "11111111",
        "11111111",
        "11111111"
    ])

    # 0x84: Distant 80s neon mountain left
    tiles[0x84] = encode_tile([
        "       *",
        "      **",
        "     ***",
        "    ****",
        "   *****",
        "  ******",
        " *******",
        "********"
    ])
    
    # 0x85: Distant 80s neon mountain right
    tiles[0x85] = encode_tile([
        "*       ",
        "**      ",
        "***     ",
        "****    ",
        "*****   ",
        "******  ",
        "******* ",
        "********"
    ])
    
    # 0x86: Retro Cloud Left
    tiles[0x86] = encode_tile([
        "        ",
        "   ***  ",
        "  ***** ",
        " *******",
        "********",
        "********",
        " *******",
        "        "
    ])
    
    # 0x87: Retro Cloud Right
    tiles[0x87] = encode_tile([
        "        ",
        "  ***   ",
        " *****  ",
        "******* ",
        "********",
        "********",
        "******* ",
        "        "
    ])

    # 0x88: Time Warp Speed Dash FX
    tiles[0x88] = encode_tile([
        "  *   * ",
        "   * *  ",
        "********",
        "   * *  ",
        "  *   * ",
        "        ",
        "********",
        "        "
    ])

    # --- Room tiles (plan.md 8a.3): tile classes by index: 0xC0-0xDF solid, others decor ---
    # 0x01-0x09: hack progress bar, 0..8 pixels filled (colour 3) over a track (colour 1)
    for k in range(9):
        band = ''.join('3' if x < k else '1' for x in range(8))
        tiles[0x01 + k] = encode_tile(["        ", "        ", band, band, band, band, "        ", "        "])

    # 0xC0 / 0xC1: solid brick (palette 0 of the room: 1 mortar, 2 brick, 3 highlight); 0xC0 has a lit top edge
    brick = ["22222221", "22222221", "22222221", "11111111", "22122222", "22122222", "22122222", "11111111"]
    tiles[0xC1] = encode_tile(brick)
    tiles[0xC0] = encode_tile(["33333333", "22222221"] + brick[2:])

    # 0x90-0x93 terminal waiting (blue screen), 0x94-0x97 hacked (green screen, dark prompt);
    # 2x2 tiles, row-major (palette 1 of the room: 1 dark blue, 2 blue, 3 green)
    def terminal(fill):
        prompt = [(1, 2), (3, 4), (5, 6), (3, 4), (1, 2, 8, 9, 10)]   # a '>' and a '_'
        art = ["                ", "  111111111111  ", " 11" + fill * 10 + "11 "]
        for cols in prompt:
            line = [fill] * 12
            for c in cols:
                line[c] = '1'
            art.append(" 1" + ''.join(line) + "1 ")
        art += [" 11" + fill * 10 + "11 ", "  111111111111  ", "     111111     ", "    11111111    ",
                "   1111111111   ", "   1  1  1  1   ", "                ", "                "]
        assert len(art) == 16 and all(len(r) == 16 for r in art)
        return art
    put_sprite(tiles, 0x90, terminal('2'))
    put_sprite(tiles, 0x94, terminal('3'))

    # Time map (src/map.c): 0xA4 path, 0xA8-0xAB era waiting (blue), 0xAC-0xAF era done (green),
    # 0xB0-0xB3 the boss (palette 2 of the room palette: 1 red, 2 orange, 3 green)
    tiles[0xA4] = encode_tile(["        ", "        ", "        ", "33  33  ", "33  33  ", "        ", "        ", "        "])
    def node(fill, ink, mark):
        art = []
        for y in range(16):
            row = ''
            for x in range(16):
                r2 = (x - 7.5) ** 2 + (y - 7.5) ** 2
                row += ' ' if r2 > 56 else ('1' if r2 > 36 else fill)
            art.append(list(row))
        for (x, y) in mark:
            art[y][x] = ink
        return [''.join(r) for r in art]
    check = [(5, 8), (6, 9), (7, 10), (8, 9), (9, 8), (10, 7), (11, 6)]          # a tick
    check = [(x, y) for (x, y) in check] + [(x + 1, y) for (x, y) in check]
    eye = [(x, y) for x in range(6, 10) for y in range(6, 10)]                    # a pupil
    put_sprite(tiles, 0xA8, node('2', '2', []))
    put_sprite(tiles, 0xAC, node('3', '1', check))
    put_sprite(tiles, 0xB0, node('1', '3', eye))

    # 0x98-0x9D gate closed (red and orange bars), 0x9E-0xA3 gate open (green arrow); 2 wide, 3 tall
    # (palette 2 of the room: 1 red, 2 orange, 3 green)
    def gate(open_):
        art = []
        for r in range(24):
            if r in (0, 23):
                art.append("1111111111111111")
            elif open_:
                inner = [' '] * 12
                if r in (9, 13): inner[5] = '3'
                if r in (10, 12): inner[5] = inner[6] = '3'
                if r == 11: inner[1:8] = ['3'] * 7
                art.append("11" + ''.join(inner) + "11")
            else:
                art.append("11" + (("22" * 6) if r % 4 in (1, 2) else ' ' * 12) + "11")
        return art
    put_sprite(tiles, 0x98, gate(False))
    put_sprite(tiles, 0x9E, gate(True))

    return b''.join(tiles)

def make_spr_table():
    tiles = [blank_tile()] * 256
    
    # Palette map (sprite sub-palette 0, see palettes.h):
    # 1 = Geeko light green
    # 2 = dark green (stripes, pupil, hoverboard stripe)
    # 3 = white (eye, hoverboard)

    # --- GEEKO (openSUSE chameleon on a hoverboard) ---
    # Drawn as whole pictures; put_sprite() slices them into 8x8 tiles, row-major.
    # '.' = transparent. Striped back (even columns light, odd dark), big white eye with a dark
    # pupil, small curled tail on the left, 2-wide legs standing on the board.
    # Both poses keep the board in rows 10..15 so it stays at the same height.

    # RIDING (16x16 = 4 tiles: 0x00..0x03)
    GEEKO_RIDE = [
        ".....2121..21...",
        "....121212.212..",
        "...212121213331.",
        "..12121212132312",
        ".212121212133312",
        "1212121212121...",
        "1212121212121212",
        "12..12....12....",
        "1.1.12....12....",
        "121.12....12....",
        "..333333333333..",
        ".33333333333333.",
        "2222222222222222",
        ".33333333333333.",
        "..333......333..",
        "...1........1..."
    ]
    put_sprite(tiles, 0x00, GEEKO_RIDE)

    # DUCKING (16x16 = 4 tiles: 0x04..0x07): body squashed onto the board, rows 5..9
    GEEKO_DUCK = [
        "................",
        "................",
        "................",
        "................",
        "................",
        "....12121212.21.",
        "..1212121213331.",
        ".212121212132312",
        "12121212121333..",
        "1212121212121212",
        "..333333333333..",
        ".33333333333333.",
        "2222222222222222",
        ".33333333333333.",
        "..333......333..",
        "...1........1..."
    ]
    put_sprite(tiles, 0x04, GEEKO_DUCK)

    # --- OBSTACLE 1: BUG / GLITCH (16x16 = 4 tiles: 0x10..0x13) ---
    # Top Left
    tiles[0x10] = encode_tile([
        " 1      ",
        "  1     ",
        "   1 111",
        "  111331",
        " 1 11331",
        "1  11111",
        "   22222",
        "1 222222"
    ])
    # Top Right
    tiles[0x11] = encode_tile([
        "      1 ",
        "     1  ",
        "111 1   ",
        "133111  ",
        "1331 1  ",
        "1111  1 ",
        "2222    ",
        "22222 1 "
    ])
    # Bottom Left
    tiles[0x12] = encode_tile([
        " 1 22222",
        "  1 2222",
        "   1 111",
        "  1   11",
        " 1    1 ",
        "1    1  ",
        "1    1  ",
        "        "
    ])
    # Bottom Right
    tiles[0x13] = encode_tile([
        "2222 1  ",
        "222 1   ",
        "111 1   ",
        "11   1  ",
        " 1    1 ",
        "  1    1",
        "  1    1",
        "        "
    ])

    # --- OBSTACLE 2: TUMBLEWEED (16x16 = 4 tiles: 0x14..0x17) ---
    # Top Left
    tiles[0x14] = encode_tile([
        "    121 ",
        "  123 21",
        " 13 12 1",
        "12 21 32",
        "2 31 21 ",
        "12 13 21",
        " 21 21 3",
        "1 32 121"
    ])
    # Top Right
    tiles[0x15] = encode_tile([
        "  12    ",
        " 12 31  ",
        "21 32 1 ",
        "1 12 21 ",
        " 23 12 2",
        "12 31 21",
        "1 21 32 ",
        "21 23 1 "
    ])
    # Bottom Left
    tiles[0x16] = encode_tile([
        "1 23 212",
        " 21 13 1",
        "12 31 21",
        "2 12 31 ",
        "13 21 12",
        " 12 31 2",
        "  12 231",
        "    121 "
    ])
    # Bottom Right
    tiles[0x17] = encode_tile([
        "12 31 1 ",
        "2 12 21 ",
        "12 31 12",
        " 21 23 1",
        "31 12 21",
        "1 32 13 ",
        "123 12  ",
        "  121   "
    ])

    # --- OBSTACLE 3: AIRBORNE FLYING DRONE (16x16 = 4 tiles: 0x18..0x1B) ---
    # Top Left
    tiles[0x18] = encode_tile([
        "        ",
        "   33333",
        "  311111",
        " 3113333",
        "32213333",
        "32222111",
        " 3322222",
        "   33333"
    ])
    # Top Right
    tiles[0x19] = encode_tile([
        "        ",
        "33333   ",
        "111113  ",
        "3333113 ",
        "33331223",
        "11122223",
        "2222233 ",
        "33333   "
    ])
    # Bottom Left
    tiles[0x1A] = encode_tile([
        "  11   3",
        " 11    1",
        "11      ",
        "11      ",
        " 11     ",
        "  11    ",
        "        ",
        "        "
    ])
    # Bottom Right
    tiles[0x1B] = encode_tile([
        "3   11  ",
        "1    11 ",
        "      11",
        "      11",
        "     11 ",
        "    11  ",
        "        ",
        "        "
    ])

    # --- FX: FIRE TRAILS & WARP PARTICLES (Tiles 0x20 - 0x21) ---
    # 0x20: BTTF Fire Trail
    tiles[0x20] = encode_tile([
        "  222   ",
        " 23332  ",
        "233*332 ",
        " 23332  ",
        "  222   ",
        " 22 22  ",
        "        ",
        "        "
    ])
    # 0x21: Crash Particle / Star
    tiles[0x21] = encode_tile([
        "   *    ",
        " * * *  ",
        "  ***   ",
        "******* ",
        "  ***   ",
        " * * *  ",
        "   *    ",
        "        "
    ])
    return b''.join(tiles)

def main():
    out_path = sys.argv[1] if len(sys.argv) > 1 else "assets/graphics.chr"
    bg = make_bg_table()
    spr = make_spr_table()
    
    total = bg + spr
    assert len(total) == CHR_SIZE, f"Expected {CHR_SIZE} bytes, got {len(total)}"
    
    with open(out_path, "wb") as f:
        f.write(total)
        
    print(f"[OK] Generated {out_path} ({len(total)} bytes, 512 tiles).")

if __name__ == "__main__":
    main()
