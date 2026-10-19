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

    # 0x89: Sprite-0 hit target. One pixel on the LAST scanline (row 7) of HUD tile row 2,
    # so the scroll split lands on the first playfield line instead of inside the HUD text.
    tiles[0x89] = encode_tile([
        "        ",
        "        ",
        "        ",
        "        ",
        "        ",
        "        ",
        "        ",
        "*       "
    ])

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

    # --- FX: FIRE TRAILS & WARP PARTICLES (Tiles 0x20 - 0x21), SPRITE 0 (0x23) ---
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
    # 0x23: Sprite 0 (hit trigger): one pixel on row 0, drawn behind the background
    tiles[0x23] = encode_tile([
        "*       ",
        "        ",
        "        ",
        "        ",
        "        ",
        "        ",
        "        ",
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
