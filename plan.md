# Hack to the Future (NES) - Game Design & Plan

Geeko (the openSUSE chameleon) races on an anti-gravity hoverboard until he hits **88 MPH**, travels through time, and **hacks the future**: in each IT era he hacks every terminal of a room to install openSUSE, avoiding era-themed foes. Final boss: **The Mainframe**, the AI of a closed-source megacorp that purges open-source systems.

**Two halves, one game:**
* **RACE**: an endless-runner road: jump and duck while the speed climbs from 20 to 88 MPH (60 s).
* **HACK**: single-screen rooms: run and jump on platforms, hold B next to a terminal to hack it, avoid foes, reach the gate.

There are no lives and no health: **one clock for the whole game (10 minutes)** is the only way to lose.

Numbers marked *proposed* are starting values to tune, not decisions. Balance numbers live in `src/tuning.h`. What is built and what is not: section 0. How the code is organised: `AGENTS.md`.

---

## 0. Status (2026-10-19)

Verified by `make test` (headless Mesen2: `tests/smoke.lua`, `room.lua`, `map.lua`, `sprites.lua`) and screenshots.

| Piece | State |
|---|---|
| Race: road, jump/duck, 3 obstacle types, 20 -> 88 MPH in about 60 s; a crash costs 10 MPH, 5 s and 1 s of invulnerability and never ends the run; blue sky | Works, tested |
| One 10-minute clock; game over only when it reaches 0, with a screen showing score and rank; A or Start restarts the game | Works, tested |
| Score: speed points, dodge streak x1 to x5, hacked terminals, clean room; ranks NEWBIE to ROOT; a flawless game is tuned to about 26,000 | Works, tested (the smoke test checks the model) |
| HUD: clock, year, multiplier, speed (race) or terminals (room), score, hi-score; MMC3 scanline IRQ splits it from the scroll | Works, tested |
| Time map before each level: eras on a line, finished ones green, Geeko blinking over the next; A or Start skips it | Works, tested |
| Warp at 88 MPH: flash, the year rolls, the room appears | Works (draft), tested |
| Room 1 (1990): run with inertia, jump, tile collision, 2 terminals (hold B), a patrolling drone, the gate, the result card; black background | Playable draft, tested piece by piece and by a bot that plays the whole room |
| One Geeko body (`geeko.c`) for the race and the rooms: the same jump, fall and run code, hitbox, drawing | Done, tested (a held jump rises 49.2 px in both) |
| FamiTone2 music (one placeholder track) and 5 effects | Works; nobody has listened to the effects in the game |
| Mapper | **MMC3 port done**: 64 KB PRG image, one 8 KB CHR tileset, IRQ split, vertical mirroring |
| Rooms for the other eras, the boss, story screens, sequence table, pause, era art, CHR banks per era | Not started |

Limits of the draft, so nobody is surprised:
* Only room 1 exists: every era plays it (the hack time follows the era: 60, 75, 90, 105 frames). The flow is hard-coded in the state functions of `main.c` (no sequence table yet, step 2.1). After the 2020 room the map shows the boss and the game ends with TO BE CONTINUED (the boss is not built).
* No "+10" / "-5" popups on the HUD; the hack tick reuses `sfx_beep` and a finished terminal reuses `sfx_warp`.
* The drone is the only foe type; its patrol code lives in `room.c`.
* Sprites per scanline, measured from OAM by `tests/sprites.lua`: race 6, drone over a ground obstacle 6, crash burst 7, warp with the fire trail 8, room 4, map 2: never above 8.
* **Cheats for testing**, each a `#define` in `tuning.h` switched before the build (both 0 in a release): `DEV_LEVEL_SELECT` (default 1: on the map LEFT/RIGHT pick any of the five levels, A starts its race, SELECT its room) and `DEV_INVULNERABLE` (default 0: Geeko is never hit and blinks all the time). `make test` needs the defaults; `tests/cheats.lua` checks both switches in both positions.
* Not tried: real hardware, a second emulator (MMC3A / MMC3B IRQ differences would matter for reload value 0, which is not used).

---

## 1. The loop

```
Built (draft):
TITLE -> MAP -> RACE (20 -> 88 MPH, 60 s) -> WARP (flash, year rolls) -> ROOM (hack 2 terminals) -> gate -> RESULT card -> MAP -> RACE ...
                  clock reaches 0 in RACE or ROOM -> GAME OVER (score, rank) -> A -> new game

Target:
        +--------------------------- repeat for each epoch ----------------------------+
        |                                                                              |
TITLE -> MAP -> RACE -> WARP (flash, year rolls) -> ROOM (hack terminals)
                                                         |
                          all terminals green -> exit gate opens -> reach it -> RESULT card
                                                         |
                                  last epoch done?  no -> MAP -> RACE ...
                                                    yes -> MAP -> BOSS: THE MAINFRAME -> ENDING (WIN)
```

* **RACE is the way into the future.** Reaching 88 MPH triggers the warp. A race lasts 60 s without crashes.
* **MAP**: before each race a time map shows the eras and where Geeko is going (8a.8).
* **ROOM is the hack level**: one single screen (no scrolling) with platforms, terminals, foes and a gate. Geeko runs with inertia and jumps (section 4a). No weapons.
* **No death.** There are no lives and no health. A hit costs **fixed time** from the clock (a race crash also slows Geeko, so reaching 88 MPH takes longer); hacking a terminal **adds time**. There is **one clock for the whole game, 10 minutes**. The only game over is the clock reaching 0; a game over restarts the whole game from the race with a full clock. Amounts: section 8a.6 and `tuning.h`.
* **Score** measures flow and speed, not the clock: section 8a.7.
* Rooms per epoch: 1 (the 10-minute clock fits that; the room data format allows several). Until each era has its own room, every era plays room 1.
* The target flow is driven by a **sequence table** (a list of `STORY`, `RACE`, `ROOM(n)`, `BOSS` rows); main.c would step through it. Not built, step 2.1.

---

## 2. Concept art (`concept-art/`)

Eleven images, named by content (renamed from `Gemini_Generated_Image_*` on 2026-10-15). The image files are not in the repo (`*.jpeg` is gitignored); they are linked from an external site. Four were inspected on 2026-10-12 (they existed as low-res `17917…jpg` plus a 2752x1536 copy; the low-res copies have since been removed) and are catalogued below. The other seven (renders from `concept-art/PROMPTS.md` and NES-style mock-ups, `nes-*` and `illustration-race-*` / `illustration-1990-*`) are identified by filename only.

| File | Shows | Design takeaways |
|---|---|---|
| `box-cartridge-manual-photo.jpeg` (a photo mock-up of box + cartridge + manual) | Box art "ゲーコング" by **GEEKO SOFT**. Geeko with goggles, thumbs up, spiral tail, sitting on a CRT between tower PCs; green binary rain over server racks; orange diagonal banner. Styled as a retro home-computer box, not NES. | Tone and palette (black, openSUSE green, orange, CRT cyan glow). A NES box/cart label would need a re-brand. Geeko wears goggles; ours does not (cheap to add) |
| `illustration-server-room-tongue-hack-guard-drone.jpeg` | **1990 stage.** Two-tier steel catwalk with railings. Geeko crouches on the upper tier and hacks a beige tower + CRT showing a green `>_ openSUSE` prompt, with cyan particles. A server at the right shows a **blue screen**. Below: a hovering **security drone** and an **armoured guard**, each with a red **vision cone**. HUD: portrait + health bar, `LIVES x3`, score, inventory slots (tower/terminal/etc. with counts), tool slots (wrench, hourglass) | Unhacked = blue screen, hacked = green terminal (cheap, readable state change). Vision cones imply stealth: foes are dangerous when you are in the cone. Two tiers fit one NES screen |
| `illustration-tail-hang-laser-grid-laptop.jpeg` | **2010 stage.** Geeko **hangs by its tail** from an overhead pipe and hacks a **laptop** on a hovering platform (openSUSE logo on screen). Hazards: red **laser tripwires** across the floor, a sparking ceiling machine. HUD: health + energy bars, lives, score, wrench + laptop slots, openSUSE badge | The ceiling hang is not in the game (jumping instead). Hazards that are not creatures (lasers, sparks) are cheap to implement and era-flavoured |
| `illustration-mainframe-boss-tongue-vs-crab.jpeg` | **Boss: THE MAINFRAME.** A monolith with a huge red slit-pupil eye, a teal swirl emblem, red circuit lines, smoke, flanked by server racks. Geeko leaps along a catwalk; its tongue is a magenta lightning bolt hitting an **orange pixel crab** (AI agent, shown glitching into green code); a second crab hovers; a cyan star sparkle. HUD: boss bar "THE MAINFRAME", player health + energy, lives, score | Boss = one static, big, readable sprite group + projectiles. The boss bar can be the count of unhacked terminals. The crab is the model for the PINCHBOT agent (8b.2) |

Conflicts between the art and this plan:
* The art hacks with the **tongue** (and uses it as a weapon on the boss). The game has **no weapons**: hack by standing in a zone and holding B. The tongue is not part of the sprite.
* The art is a side-scrolling platformer with health bars; the plan has no health and no lives (decision 2026-10-19: no death, only the clock ends the game).

---

## 3. Epochs (all values *proposed*)

| # | Year | Terminal look | Terminals | Hack time | Foes (concept art in **bold**) | Hazards |
|---|---|---|---|---|---|---|
| 1 | 1990 | beige tower + CRT | 2 | 60 frames | **security drone**, **armoured guard with flashlight cone**, floppy disk frisbees | loose cables |
| 2 | 2000 | beige desktop + flat LCD | 3 | 75 | Y2K bug, popup windows, a paperclip-style helper | falling CRTs |
| 3 | 2010 | laptops (on hovering platforms) | 3 | 90 | malware padlock, "updating..." hourglass blob | **laser tripwires**, **sparking ceiling machine** |
| 4 | 2020 | phones / tablets / cloud racks | 4 | 105 | phishing hook, bot swarm | moving platforms |
| B | the AI era | the Mainframe's 6 terminals | 6 | 120 | The Mainframe + **crab AI agents** | - |

* Moods: 1990 UNIX labs / BBS (warm red brick, CRT glow); 2000 dot-com server farm (steel racks, blue LEDs, firewalls); 2010 cloud (white cleanroom panels, cyan fibre); 2020 AI edge (dark chassis, neon green traces); the boss is the 2054 AI Citadel (obsidian, neon green).
* Era entry (as built for the first one): warp flash, the HUD year rolls 1985 -> 1989 and the room is drawn at the flash's peak, showing 1990.
* Foes are era-flavoured re-skins of 4 behaviours, so code stays small: **patrol** (walks a platform), **hover** (sine path), **stationary hazard** (laser, spark, energy field), **projectile**. New foe = new tiles, not new code. Built so far: the drone's patrol.

---

## 4. Hacking mechanic (as built)

* Stand on the terminal's surface with Geeko's centre inside its zone (the terminal's two columns plus one column on each side) and **hold B**. The D-pad is ignored then, so Geeko slides to a stop; the progress counts frames **while Geeko is standing still**. Frames needed: `HACK_FRAMES_1990` 60, `_2000` 75, `_2010` 90, `_2020` 105, `_BOSS` 120 (`tuning.h`).
* Progress is a 4-tile bar one row above the terminal, 32 steps, drawn with the common tiles `0x01-0x09` through the VRAM buffer; `sfx_beep` every 8 frames. Geeko takes the duck pose while hacking.
* Releasing B or leaving the zone resets the progress and clears the bar. A foe touch knocks Geeko back, which also leaves the zone.
* Finishing a terminal: it turns green (tiles swapped), `sfx_warp`, `clock_add(HACK_TIME_BONUS_S)`, `score_add(TERMINAL_POINTS)`.
* When every terminal is green the **gate opens** (tiles swapped); touching it ends the room: +`CLEAN_ROOM_POINTS` if Geeko was never hit, then the result card, then the next RACE.
* Not built: a "+10" popup, a dedicated tick sound.

---

## 4a. Room movement (as built, `hero.c`)

Geeko moves like a platformer hero with the hoverboard's **inertia on start and stop**. The code is **one body for every level** (`geeko.c`: `geeko_jump`, `geeko_fall`, `geeko_run`, `geeko_box`, `geeko_draw`, `struct Geeko`): the race (`player.c`) adds a flat road, ducking and the bob; a room (`hero.c`) adds tile collision and knockback; the boss will use `hero.c` too. So the race has the same variable-height jump (a tap hops 13 px, a held jump rises 49 px; `tests/smoke.lua` and `tests/room.lua` measure the same 49.2 px). 8.8 fixed point; measured by `tests/room.lua`:

| | Ground | Air |
|---|---|---|
| Run acceleration | +0x18 per frame to a max of 0x200 (2 px/frame); **measured 23 frames to full speed** | +0x0C (weaker steering) |
| No input | friction -0x10 per frame; **measured 33 frames and 31 px to a stop** (the glide) | momentum kept |
| Reversing | skid: -0x30 per frame while the board turns | steer at the air rate |
| Jump | A or Up: impulse -0x4C0; letting go of the button while rising cuts the upward speed to 0x180 (a tap gives a short hop); **measured apex 49.2 px** | gravity 0x38 per frame, fall speed capped at 0x700 (under one tile per frame, so no tunnelling) |

* **Collision** per axis against `room_solid()` (the room's rectangles): hitbox x+2, width 12, y+1, height 15 inside the 16x16 sprite; the left and right screen edges are walls; standing is probed at three points below the feet.
* **Facing left:** a second pair of metasprites with the flip attribute (`GeekoRideL`, `GeekoDuckL`), no extra CHR. The hack pose is the duck pose.
* **Knockback** (a foe touch): 10 frames at 2 px/frame away from the foe, a small hop, D-pad ignored; then invulnerability (`INVULN_FRAMES`, Geeko blinks).
* Controls in rooms: D-pad left/right = run, A (or Up) = jump, B = hold to hack. Down and Start are unused (pause not built; coyote time not built).

---

## 5. Boss: THE MAINFRAME

Not built. The full design is section 8b; in short: a single-screen arena with 6 terminals on 3 tiers (left and right), a Mainframe with a big eye as background art, AI agents (PINCHBOT, WHIRLY, TWINKLE) that fly along lanes, no weapons (agents can only be avoided), each hacked terminal closes one lane and gives time, the sixth triggers an overload sequence and the WIN screen.

---

## 6. HUD (as built, `hud.c`)

Two text rows in nametable rows 1-2, above the MMC3 scroll split (rooms and the map do not scroll):

```
TIME 10:00  YEAR 1985  X1
SPD:20MPH  SCR:00000 HI:00000        (race)
HACK 0/2   SCR:00000 HI:00000        (room)
```

Digit columns: clock 6-10, year 18-21, multiplier 25; speed 5-6 or terminals hacked 6 / total 8; score 16-20; hi-score 25-29. The clock shows the seconds rounded up (`10:00` until a full second has passed). `hud_update()` writes only the fields that changed, because the NMI can flush only about 100 bytes of VRAM buffer per vblank. The warp rolls the year field (`hud_year_show()`).

Proposed for the boss: top row replaced by `THE MAINFRAME` and one `#` per unhacked terminal (8b.3).

---

## 7. NES constraints and what they force

1. **CHR budget (the biggest issue).** The ROM has one 8 KB CHR tileset so far: 256 background tiles and 256 sprite tiles. Today's BG table holds the font, the race scenery (`0x80-0x88`), the room tiles (hack bar `0x01-0x09`, terminals `0x90-0x97`, gate `0x98-0xA3`, brick `0xC0-0xC1`) and the map (`0xA4-0xB3`); sprites hold Geeko, obstacles and FX. Four eras plus a boss with distinct terminals, platforms and foes will not fit: bank the CHR **per era** with the MMC3 (the warp flash is where to swap; step M3). Mapper: section 7a.
2. **Sprites per scanline: 8.** Measured from OAM by `tests/sprites.lua`: race 6, a drone over a ground obstacle 6, the crash burst 7, the warp with the fire trail 8 (the trail only exists there: the race ends at 88 MPH), a room 4, the map 2. A room with three 2-tile foes would be 8; the boss with 3 agents is exactly 8. Draw order puts the least important sprites last.
3. **Level data.** Room 1 is 4 rectangles, 2 terminals, 1 gate and 1 foe: tens of bytes, generated into `assets/rooms.h`. RAM-free (`const` in PRG).
4. **RAM 2 KB:** about 0.9 KB in use (zero page 223 B, `.aligned` 416 B including the 256 B OAM buffer, `.bss` 68 B, `.noinit` 187 B; the C software stack is not counted). Room layouts, foe lists and text stay `const` in PRG.
5. **PRG:** 16.9 KB used (`.text` 14.9 KB, `.rodata` 2.0 KB including the audio data) of the 64 KB image; the code sits at `$A000` and up.
6. **VRAM buffer:** about 100 bytes per vblank. Bigger updates need `ppu_off()` (the game-over screen and the result card do that).
7. **Audio:** FamiTone2, section 7b.
8. **Testing.** Mesen Lua can poke RAM, so tests jump straight to a state (`mph`, `time_left`, `score`, the hero position), read the PPU (nametable text, tiles) and the APU. `make test` runs `tests/smoke.lua` and `tests/room.lua`; `tests/lib.lua` holds the shared helpers and reads `src/tuning.h`.

---

## 7a. Mapper alternatives in the current compiler

Checked 2026-10-12 in the llvm-mos SDK of `Containerfile` (v23.3.0): targets `mos-nes-<mapper>-clang` and their linker scripts under `mos-platform/nes-<mapper>/lib`. Limits below are the SDK's own `ASSERT`s, not the hardware's.

| Target (mapper) | CHR | PRG | What it gives us | Notes |
|---|---|---|---|---|
| `nrom` (0) | up to 8 KiB ROM or RAM | fixed | simplest | too small for per-era art |
| `cnrom` | CHR-ROM in **8 KiB banks**, up to 2048 KiB | fixed (no PRG banking) | `set_chr_bank(n)`; `swap_chr_bank(n)` / `split_chr_bank(n)` change the bank at the next NMI / until the next NMI. BG and sprite tables swap together | Lowest-effort fix. Each bank repeats the font + HUD tiles (~50). Bus conflicts handled by the SDK's rompoke table (`_rompoke.ld`) |
| `mmc1` | CHR-ROM up to 128 KiB in **4 KiB banks** (BG and sprites independent), or CHR-RAM up to 8 KiB | banked (`set_prg_bank`, `banked_call`) | independent BG / sprite banks, PRG banking, PRG-RAM up to 32 KiB | More code than CNROM; needs `banked_call` discipline |
| `mmc3`, **current** | CHR-ROM in **1 KiB banks** (finest) | up to 512 KiB, banked (`set_prg_8000/a000`, `banked_call*`) | per-1 KiB CHR swaps and a **scanline IRQ** (`set_irq_ptr`, `is_irq_done`), so the HUD split would not need sprite 0; PRG-RAM up to 8 KiB, mirroring control | Most capable and the most complex; widely supported by emulators and flash carts (the `file-fixers` sibling uses it) |
| `unrom` | 8 KiB (in practice CHR-RAM) | banked, up to 4096 KiB | big PRG; with CHR-RAM, tiles are uploaded from PRG at warp time (screen off) | Art lives in PRG, so no CHR-ROM size limit, but needs an upload routine |
| `unrom-512` | ROM or RAM up to 32 KiB | up to 512 KiB, banked | mirroring control, homebrew-board friendly | CHR-RAM like UNROM |
| `gtrom` | CHR-RAM up to 16 KiB | up to 512 KiB (32 KiB banks) | 2 nametable banks, board LEDs | Homebrew board |
| `action53` | up to 32 KiB (ROM or RAM) | banked (inner/outer) | multicart mapper; `mbh-A53-witchnwiz` uses it | Aimed at compilations |
| `mos-nes` (generic) | - | - | base platform with no mapper code | Not a game target |

All of them link the same `neslib`, `nesdoug` and FamiTone2 libraries. A different mapper changes the compiler command (`CC=mos-nes-<mapper>-clang`), the linker section names for CHR (`chr_rom_0`, `chr_rom_1`, ... for banks) and the iNES header; `build_chr.py` / `chr-rom.s` would emit one bank per epoch. Not prototyped: bank section names for `.incbin` and the bus-conflict handling still have to be tried.

**Decided 2026-10-19: MMC3, and built the same day (steps 1.1-1.3).** Only MMC3 has a scanline IRQ among the eight SDK targets (the other `mapper.h` files have none), so it is the only one that removes the sprite-0 trick; it also has 1 KB CHR banks, so the font and Geeko tiles are not repeated per era.

**Spike (2026-10-19, `/tmp/mmc3-spike`, a throwaway copy, not in the repo)**: the current game built with `mos-nes-mmc3-clang` and ~30 changed lines (init in `main.c`, `hud_scroll()` building the IRQ array, sprite 0 disabled). Measured in Mesen:
* `tests/smoke.lua` passes unchanged, including the scroll split on scanline 23. IRQ array `{22, 0xF0, ctrl, 0xF5, x, 0xFF}`: the IRQ handler's two `$2005` writes land on scanline 23, dots 35-57, before dot 257, so the new coarse scroll applies from line 24. With count `23` they land on line 24 (the first playfield line, black sky: invisible here, but a glitch for other art).
* Game logic starts at scanline 3 instead of 29 (`scroll_x` write observed over 59 frames in each ROM; MMC3: scanline 2-4 in most frames, up to 12 in a few): about 26 scanlines (roughly 3,000 cycles, 10% of a frame) are no longer spent spinning on the sprite-0 flag. On NROM the spin ran from scanline 258 to 23.
* Default ROM image is **768 KB** (header: 512 KB PRG, 256 KB CHR). `MAPPER_PRG_ROM_KB(64); MAPPER_CHR_ROM_KB(8);` (from `ines.h`) gives 73,744 bytes, header PRG 4 x 16 KB, CHR 1 x 8 KB, mapper 4, and all tests pass. The link script asserts PRG greater than 32 KB.
* `set_chr_bank` "may have no effect if interrupted"; use `set_chr_bank_retry` outside vblank-safe code. The IRQ array has no CHR commands, so main-thread CHR changes are safe (comment in `nesdoug-llvm/33_MMC3/MMC3/mmc3_code.asm`).
* Not checked: real hardware (MMC3A / MMC3B IRQ differences matter for reload value 0, not used here), other emulators, how the SDK restarts the IRQ array each frame (observed to work, source not read).

**The port** changed `main.c` (`mapper_init()`, the ROM-size macros), `hud.c` (the IRQ array), `render.c` and `screens.c` (sprite 0 gone), `build_chr.py` (its two tiles gone) and the Makefile (`mos-nes-mmc3-clang`); every test passed unchanged. The ROM is 73,744 bytes. Not checked: real hardware, other emulators, how the SDK restarts the IRQ array each frame (observed to work, source not read).

---

## 7b. Music and sound effects

**What we have today (draft since 2026-10-16).** FamiTone2 plays one track and the five effects: `sfx.c` calls `music_play` / `sfx_play`, the data is in `assets/audio/` (generated by `tools/export_audio.sh`, see below). The old raw APU pokes are gone (the engine rewrites every channel each frame; the raw crash noise had also rung forever, fixed by the move). Draft limits: the track is `nes_23-chill`, a placeholder, with its volume-column echo lost (FamiTone2 has no volume column; `/tmp/music-listen/chill_famitone2_approx.wav` is FamiStudio's approximation of what plays); the music starts with a new game, keeps playing through the race, the warp and the room, and stops at game over; the title screen is silent; the effects were redrawn in FamiStudio text (`assets/audio/sfx.txt`): jump = rising chirp (Pulse 1), duck = falling chirp (Pulse 1), beep = high ping (Pulse 2: multiplier up, the hack tick every 8 frames, game start), warp = rising whoosh (Triangle: the warp and a finished terminal), crash = noise burst that fades in under a second (Noise: every hit). Nobody has listened to the effects in the game yet. The warp has no music change.

**What the current stack already includes** (checked in the container, 2026-10-12):
* **FamiTone2** (Shiru): `neslib.h` includes `famitone2.h`; `libfamitone2.a` is in the SDK. API: `music_init` / `music_play(n)` / `music_stop` / `music_pause`, `sounds_init` / `sfx_play(id, channel 0..3)`, `sample_play(n)` for DPCM, and `banked_music_init` / `banked_sounds_init` for banked mappers. `nesdoug` adds `set_music_speed`. The examples never call an update function, so the engine is driven from the NMI (inferred).
* **Converters in the container**: `ft2-text2data` (FamiTracker text export -> music data) and `ft2-nsf2data` (NSF -> sound-effect data), "famitone version 2d". Linux-native, no Wine.
* **DPCM**: `.dmc` samples and a linker script `dpcm.ld` (samples must sit late in ROM, at or above `$C000`).

**Verified workflow** (reproduced here against the sibling `nesdoug-llvm/18_Music`): compose in **FamiTracker** (or Dn-FamiTracker) -> export text `song.txt` -> `ft2-text2data song.txt -ca65` -> `sed 's/@/.L/g'` (LLVM assembler wants `.L` local labels; with it the output is byte-identical to `nesdoug-llvm`'s checked-in `TestMusic3.s`) -> `.s` file included from a small `music.s` that defines `music_data` in `.rodata.music_data` (same for `sounds_data` from the NSF) -> `music_init(music_data); sounds_init(sounds_data); music_play(0); sfx_play(id, 0);`. Link with `-lfamitone2`. Sound effects must stay small (255 bytes each), and DPCM cannot be used inside SFX.

**Decided:** FamiTone2 (bundled with the SDK, works in this toolchain, reference code in `nesdoug-llvm` chapters 18-20) with FamiStudio as the converter (below). Alternatives not pursued: FamiTone 5.0 (own Windows converters, run under Wine by two of the sibling projects, format not verified compatible), the FamiStudio engine (cc65 assembly, needs porting to LLVM assembler syntax), a custom FamiTracker driver (same porting problem), and raw APU pokes (cannot coexist with an engine on the same channels).

**What the other NES projects in the folder use**

| Project | Engine and tools | Mapper / compiler |
|---|---|---|
| `nesdoug-llvm` | FamiTone2 via llvm-mos: music, SFX and DPCM chapters | NROM, MMC1, MMC3; llvm-mos |
| `from-below` | FamiTracker `.ftm` -> `text2data.exe` / `nsf2data.exe` (FamiTone2) -> ca65; two music tracks | NROM-32K, cc65 |
| `mbh-A53-witchnwiz` | FamiTone 5.0 (`famitone5.s`, `text2vol5.exe`, `nsf2data5.exe` under Wine) plus `famitone2.s`, DPCM samples | Action53, cc65 |
| `file-fixers` | FamiTone 5.0 converters + FamiTracker, called from the Makefile | MMC3, cc65 |
| `famidash` | FamiStudio 4.4.2+ engine; Dn-FamiTracker modules, own export scripts, DPCM | cc65, large game |
| `nes-starter-kit` | FamiTracker: music as a "BIN raw" export with a custom driver, SFX through FamiTone2 | cc65 |
| `MK1_NES` | neslib + FamiTone (Shiru), per its README | cc65 |

The licence of each project is in section 10.

**Music sources (free to reuse)** (web search 2026-10-16; licences taken from the listing pages, not verified beyond that; re-check each page and keep the download URL on record before shipping):

| Source | Licence | Notes |
|---|---|---|
| [Free NES Music Pack](https://opengameart.org/content/free-nes-music-pack) (OpenGameArt, pmiller) | CC0; its `read_me.txt` says free for anything, with or without attribution | 10 `.ftm` + 1 DefleMask (+ 49 MB of WAV). **7 are 2A03 only, tempo 150** (FamiTone2's fixed tempo): `nes_02-doom`, `03-blues`, `07-jazz`, `07-simple`, `11-soft_then_loud`, `19-upbeat`, `23-chill`. **3 need VRC6**: `02-incomplete`, `03-cheery`, `05-hectic`. All 7 still use volume-column cells and most use effects; closest to ready: `07-simple` (2 volume cells, few effects), `19-upbeat` and `23-chill` (no effects, but ~150 volume cells). Some tracks need loop editing |
| [Lasso Lady (seamless loop)](https://opengameart.org/content/lasso-lady-seamless-loop) (OpenGameArt, congusbongus) | CC0 | `lassolady.zip` (2 KB) holds `lassolady.ftm`: **2A03 only**, 5 frames x 192 rows, speed 6, tempo 140, 3 `Bxx` jumps, 17 volume-column cells, a few effects. **Closest to FamiTone2-ready of everything checked.** Tempo 140 is not 150: FamiTone2 plays it a bit fast unless the speed is reworked (from memory of how tempo works, not tested) |
| [Slam-Funk](https://opengameart.org/content/nes-chiptune-slam-funk) (OpenGameArt, Haley Halcyon) | **CC-BY 4.0**: credit "Slam-Funk" composed and arranged by Haley Halcyon (needs a credits screen) | `funk.7z` (3.7 KB) holds the `.ftm`: **2A03 only**, 8 frames x 64 rows, speed 6, **tempo 122** (needs a speed rework for FamiTone2), 34 effect cells, 80 volume-column cells |
| [NES Shooter Music](https://opengameart.org/content/nes-shooter-music-5-tracks-3-jingles) (OpenGameArt, SketchyLogic) | CC0 (tagged) | **All 8 `.ftm` use VRC6** (8 channels; the page does not say so) and most use DPCM: **not usable on NROM/CNROM** without a 2A03 rework. Tracks Boss, Venus, Map, Mars, Mercury + IntroJingle, WarpJingle, WinJingle. **Risk:** comments mention a disputed third-party DMCA claim (a Steam takedown on a game that used them); read the thread before use |
| [Dark Music (VRC6 and 2A03)](https://opengameart.org/content/dark-music-vrc6-and-2a03) (OpenGameArt) | CC0 | Uses VRC6: does not fit NROM/CNROM, would need a 2A03-only rework. FamiTone2 has no expansion-chip support (from memory, not checked here) |
| [Famitracker Audio](https://opengameart.org/content/famitracker-audio), [CC0 - Retro Music](https://opengameart.org/content/cc0-retro-music) (OpenGameArt collections) | Varies per item | Index pages only: check each track's licence and whether the `.ftm` is attached |
| [Ozzed.net](https://ozzed.net/music/) | Varies per album | At least one album is explicitly not CC. Check each album |
| [Creazilla](https://creazilla.com/search/audio/famitracker) | Public domain | Audio files only, no `.ftm`: no use for FamiTone2 |
| [Pulsar [Famitracker 2A03]](https://opengameart.org/content/pulsar-famitracker-2a03) (OpenGameArt) | CC0 | WAV only, no `.ftm`: no use for FamiTone2 |

**FamiStudio as the converter (decided 2026-10-16; tested the same day).** FamiStudio 4.5.3 (`FamiStudio453-LinuxAMD64.zip`, SHA-256 `9133afed2fb384663b0735d8e950cdc2714849f4fd476b16de8d56e5e81da0ad`) reads FamiTracker 0.4.6 `.ftm` directly and exports FamiTone2 data headless. Recipe (nothing installed on the host; scratch files in `/tmp/famistudio`): a container `FROM mcr.microsoft.com/dotnet/runtime:8.0` plus `libgtk-3-0 xvfb xauth libgl1 libglu1-mesa libopenal1 libx11-6 libxcursor1 libxrandr2 libxi6 libxinerama1`; then `xvfb-run -a dotnet FamiStudio.dll song.ftm famitone2-asm-export song.s -famitone2-asm-format:ca65`. It needs a display (GTK starts even for `-help`), hence Xvfb; the openSUSE `bci/dotnet-runtime` image does not exist on registry.opensuse.org. The output has the same layout as `ft2-text2data`'s (`.byte` song count, instrument and sample pointers, five channel pointers and the two tempo words per song), but uses ca65 `@` labels and `.export`: it needs the same `sed 's/@/.L/g'` plus removing the `.export` / `.global` lines for the LLVM assembler (done for `chill`; it assembles and plays, see below).

The other route, FamiStudio `famitracker-txt-export` then `ft2-text2data`, **does not work** on these files: the converter stops at the first error (`Instrument N does not have volume envelope` on 6 tracks, `Unsupported effect` on Lasso Lady, `Unexpected character in the note field` on Slam-Funk). The direct FamiTone2 export fills the missing envelopes itself.

**Test results (2026-10-16, FamiTone2 export, sizes are the whole `.s` file):**

| Track | Result |
|---|---|
| `lassolady` | 825 B. 2 warnings: effect `7` unsupported, ignored |
| `nes_23-chill` | 372 B. 1 warning: orphan stop note at the loop point |
| `nes_07-simple` | 279 B. Same warning |
| `nes_19-upbeat` | 647 B. No warnings |
| `nes_02-doom` | 667 B. No warnings |
| `nes_11-soft_then_loud` | 878 B. 1 warning (loop point stop note) |
| `nes_07-jazz` | 2263 B (the biggest). No warnings |
| `funk_opengameart` (Slam-Funk) | 682 B. 3 warnings; one (a slide that ends on a release note) needs a manual fix |
| `nes_03-blues` | **FamiStudio cannot import it** (`Failed to create instrument 8`) |
| VRC6 tracks | exported but meaningless on a 2A03; skipped |

Not yet done, so none of this is proven by ear or by the engine: (1) only `nes_23-chill` was played by the SDK's FamiTone2 (next paragraph); the other exports were not; (2) the "no volume column / effects" and "tempo 150" limits were not checked against what FamiStudio did to them: it ignores unsupported effects with a warning, so a track can export cleanly and still sound different; (3) the original parser used for the earlier counts (`.ftm` header read from `/tmp`, effect letters from memory) is only a pre-filter.

**Played through the SDK's FamiTone2 (2026-10-16, throwaway ROM in `/tmp/music-rom`, not in the repo).** `chill.s` = the FamiStudio export with `sed 's/@/.L/g'` and the `.export` / `.global` lines deleted, included from a `music.s` that defines `music_data` in `.rodata.music_data` (same wrapper as `nesdoug-llvm/18_Music`); `main.c` = `music_init(music_data)`, 60 frames of `ppu_wait_nmi()`, `music_play(0)`, then an NMI wait loop (no update call: the engine runs from neslib's NMI). Build: `mos-nes-nrom-clang -Os -I. main.c music.s chr-rom.s -o music_test.nes -lneslib -lnesdoug -lfamitone2` in the `llvm-mos` container. A Mesen Lua script logged every write to `$4000-$4017` for 3000 frames (50 s) and read the emulated APU state: before `music_play` every volume write is silent and the triangle period is constant; after it, pulse 1, pulse 2 and noise write audible volumes, all three period registers and the APU's own timer periods keep changing, and pulse 1 is still audible after the song's ~37 s length. So the exported data drives the real engine and the song keeps going. **Not shown:** that it sounds like the original (nobody listened; no spectrum or note-by-note comparison), nor that unsupported effects dropped by FamiStudio did not change the tune. Note for tests: FamiTone2 writes all channel registers every frame, even before a song starts, so "no writes" is the wrong control; compare the written values.

Rules: GPL-3.0 repo (section 10) takes CC0 fine. Reused tracks must fit FamiTone2 (2A03 only, effect subset) and the ROM budget. Original era music (section 8, step 8.2) avoids the licence question; reused CC0 tracks serve as placeholders (the game already plays `chill`).

---

## 8. Roadmap (step by step)

**Mapper: MMC3, ported 2026-10-19** (section 7a). The milestones are ordered so the game is **playable after every milestone**; the cut lines at the end say where to stop if time runs out. Hack Week 26 is 2026-11-30 to 12-04. Much of M2 and M4 was built ahead of M1 and M3, on NROM: the MMC3 port does not block any of it. State column: **Done** (built and tested), **Draft** (works, to be finished), **Open**.

Rules for every step: it ends with `make test` green; logic gets a check (`tests/smoke.lua`, `tests/room.lua` or a script next to them); a change that must not alter the picture is checked with `tests/trace.lua` (diff identical); one `DIARY.md` line; commit only when asked. Sizes are relative (S, M, L), not time.

**Budgets** (MMC3 build, 2026-10-19; re-measure with `llvm-readelf -S -W hacktothefuture.nes.elf`):

| Resource | Now | Limit / target |
|---|---|---|
| RAM | about 0.9 KB (section 7) | 2 KB. Room layouts, foe lists and text stay `const` in PRG |
| PRG | 16.9 KB (`.text` 14.9 KB + `.rodata` 2.0 KB, audio data included) | the 64 KB image (`MAPPER_PRG_ROM_KB(64)`); where the default code placement ends is **not checked**: when `.text` + `.rodata` pass about 20 KB, check it and move room data and story text to banked sections (`#pragma clang section`, as in `nesdoug-llvm/33_MMC3`) |
| CHR | 8 KB | 32 KB target: 4 KB common + 4 KB per era (M3) |
| CPU | game logic starts at about scanline 3 (29 before the MMC3 port: the sprite-0 spin ran from scanline 258 to 23) | watch the "main loop keeps up with 60 fps" check |

### M1. MMC3 port (platform)

| # | Step | Done when | Size | State |
|---|---|---|---|---|
| 1.1 | Port as in the spike: `main.c` `mapper_init()` (`disable_irq`, `CLI`, PRG mode 0, `CHR_A12_INVERT`, 6 CHR banks, `MIRROR_VERTICAL`, IRQ pointer); `hud_scroll()` builds `{22, 0xF0, ctrl, 0xF5, x, 0xFF}` (`ctrl` = `PPU_CTRL_BASE` from `game.h` plus the nametable bit); `MAPPER_PRG_ROM_KB(64)`, `MAPPER_CHR_ROM_KB(8)` | `make test` unchanged and green; ROM is 73,744 bytes | S | Done |
| 1.2 | Delete sprite 0: `hud_sprite_zero`, `hud_draw_split_target`, the OAM reserve, tiles `0x23` / `0x89` | `make test` green | S | Done |
| 1.3 | `CC` default and `make podman` use `mos-nes-mmc3-clang`; AGENTS.md, README.md and this plan describe the IRQ split | docs match code | S | Done |
| 1.4 | Run the ROM in a second emulator (Nestopia, `AGENTS.md` Run) and look at the HUD; real hardware if a cart is available | no garbled HUD row (only the user can judge this; Mesen is the only emulator checked) | S | Open |

### M2. Game loop skeleton

| # | Step | Done when | Size | State |
|---|---|---|---|---|
| 2.0 | Test harness: `tests/lib.lua` (check, wait, press, poke, symbols from the `.mlb`, `tuning("NAME")` reading `src/tuning.h`, nametable and APU helpers, the runner); `make test` runs `smoke.lua` and `room.lua` | same checks as before | S | Done |
| 2.1 | Sequence table: `struct Step {kind, arg}` rows (`STORY`, `RACE`, `WARP`, `ROOM`, `BOSS`, `WIN`) and a `seq_index`; `main.c` runs the current row (today the flow is hard-coded in the state functions) | a test pokes `seq_index` to jump to any row | M | Open |
| 2.2 | Font: `'` and `,` are in; add what the story text needs and decide lowercase (D3; it would sit at `0x61-0x7A`, in a common CHR bank) | the glyph test scans every screen state, not only the title | S | Partial |
| 2.3 | Race without death: `mph` rises by itself (+1 per `RACE_FRAMES_PER_MPH` = 53 frames, 60 s from 20 to 88); a crash costs `CRASH_MPH_LOSS` and starts `INVULN_FRAMES`; `speed_px` follows `mph` | smoke test: a crash keeps the game going, costs the MPH, no second hit while invulnerable, a clean race reaches 88 MPH in 3,500-3,700 frames | M | Done |
| 2.4 | Clock: `time_left` (u16 frames, `GAME_TIME_S` = 600), `clock_add/sub/tick`, HUD `TIME mm:ss`, crash and contact cost `HIT_TIME_PENALTY_S`, hacks give `HACK_TIME_BONUS_S`; at 0 the game-over screen (score, rank, PRESS A TO RESTART) | smoke test: the clock starts full, a crash costs exactly the penalty, time up ends the game, the screen shows score and rank, A restarts with a full clock | M | Done |
| 2.5 | Warp at 88 MPH: 62 frames: the palette brightens (`pal_bright`), the HUD year rolls 1985-1989 and `sfx_warp` plays, at frame 30 the room is drawn at full white, then it fades back; obstacles frozen; the clock is stopped | smoke test: 88 MPH starts the warp, it ends in the room showing 1990 | M | Draft (one fixed year pair; no year table) |
| 2.6 | A room after the warp | room 1 replaces the planned placeholder | S | Done |
| 2.8 | Time map before each race (8a.8): eras on a line, finished ones green, Geeko blinking over the next, A or Start skips after `MAP_SKIP_FRAMES`, else it ends after `MAP_FRAMES`; the epoch (era counter) lives in `progress.c`; the boss's map ends the game with TO BE CONTINUED | `tests/map.lua`: the map, the blinking, the skip rules, the era progress, the ending | M | Done |
| 2.7 | Story screens: text-only renderer `story_show(id)` and a text table (intro, one per epoch, pre-boss, ending) | each screen appears in sequence and Start advances; glyph test | M | Open |

### M3. Era graphics foundation (MMC3 banking)

With `CHR_A12_INVERT`: R0 = `$1000-17FF` (2 KB), R1 = `$1800-1FFF` (2 KB), R2 `$0000-03FF`, R3 `$0400-07FF`, R4 `$0800-0BFF`, R5 `$0C00-0FFF` (1 KB each; from `mmc3_code.asm`). Current tiles map onto this: font `0x20-0x5F` is in R2 and R3 (fixed, common); race scenery and room decor `0x80+` are in R4 and R5 (the era banks: today brick `0xC0-0xC1` is in R5); Geeko, obstacles and FX `0x00-0x23` are in R0 (fixed, common). **Per era: R4 + R5 (BG tiles `0x80-0xFF`) and R1 (sprite tiles `0x80-0xFF`)**, three register writes. About 64 BG tiles stay free in the common banks (`0x0A-0x1F`, `0x60-0x7F`).

| # | Step | Done when | Size | State |
|---|---|---|---|---|
| 3.1 | `build_chr.py` emits 1 KB banks with a named layout (common BG, common sprites, era N BG, era N sprites); `chr-rom.s` includes the bigger file; `MAPPER_CHR_ROM_KB` raised | `graphics.chr` size asserted; ROM boots | M | Open |
| 3.2 | `chr_set_era(n)`: the three writes, done during the warp flash. Use `set_chr_bank_retry` (a plain `set_chr_bank` "may have no effect" if interrupted; the HUD IRQ shares the register). Safe because the HUD IRQ array has no CHR commands | test: tile `0x80` read through `nesPpuMemory` differs between era 0 and era 1 after the warp | S | Open |
| 3.3 | Two throwaway era tilesets (recolour of the current one) to prove the swap in a screenshot | screenshot differs; palette per era in `palettes.h` | S | Open |

### M4. Room engine

Design of the rooms themselves (maps, foes, numbers): section 8a. Modules: `room.c` (data, loading, terminals, gate, drone), `hero.c` (Geeko in rooms; `player.c` stays the race). A room is one screen (32x30 tiles), HUD in rows 1-2, no scroll.

| # | Step | Done when | Size | State |
|---|---|---|---|---|
| 4.1 | Room data in PRG (`struct RoomDef` in the generated `assets/rooms.h`: solid rectangles, terminals, gate, start, foes). **As built: a list of rectangles drawn at load, with `room_solid()` reading the same list**, not a raw 960-byte nametable (tens of bytes per room instead of 1 KB; revisit if a room needs decor). Solid tile class `0xC0-0xDF` (8a.3) is for the art | room 1 is drawn and Geeko collides with it | M | Done |
| 4.1a | `tools/check_rooms.py` (rooms as rectangles, reachability check against the 8a.2 rules, ASCII maps) and `tools/build_rooms.py` (generates `assets/rooms.h`, refuses an unreachable room); a Makefile rule rebuilds `rooms.h` | `python3 tools/check_rooms.py` passes for the five rooms; the build generates room 1 | M | Done |
| 4.2 | `room_load(id)`: `ppu_off`, room palette, rectangles, terminals, gate, attribute table, HUD text, Geeko placed | `tests/room.lua`: HUD, floor, terminals and gate tiles, start position | S | Done |
| 4.3 | `hero.c` horizontal movement with inertia (accel `0x18`, max `0x200`, friction `0x10`, skid `0x30`) | measured: 23 frames to full speed, 33 frames and 31 px to stop | M | Done |
| 4.4 | Gravity, jump with variable height, tile collision per axis (floor, walls, ceiling), falling off ledges; the jump, fall and run code is `geeko.c`, shared with the race | tests: apex 49.2 px (race and room), a tap is shorter, the head hits a platform, landing on a platform, the screen edge is a wall | L | Done |
| 4.5 | Facing: left-facing metasprites (flip attribute `0x40`), hack pose = the duck pose | screenshots | S | Done |
| 4.6 | Terminals: zone, **hold B** hack meter (4-tile bar through the VRAM buffer), D-pad ignored while holding, release resets, tick every 8 frames (`sfx_beep`), completion gives time, points and `sfx_warp` | tests: release resets, 60 frames hack it (+10 s, +100 points, tiles turn green) | M | Done (no dedicated sound, no popup) |
| 4.7 | Gate: opens when every terminal is hacked (tile swap); touching it ends the room, shows the result card (race time, terminals, clean-room bonus or none, score, rank), then A starts the next race | tests: the gate stays shut with one terminal left, opens with two, the card shows | S | Done |
| 4.8 | Room 1 (1990) playable end to end | a bot plays the whole room with the pad (10.5 s, no hits, 450 points); era-specific art (catwalk, tower, CRT) not drawn: brick, a terminal and a gate tile set | L | Draft |
| 4.9 | Pause on Start | test: state frozen while paused | S | Open |

### M5. Foes and hazards

| # | Step | Done when | Size | State |
|---|---|---|---|---|
| 5.1 | `foe.c`: up to 4 slots, `struct Foe` with `x, y, width, height` first (for `check_collision`), the 4 behaviours of 8a.4 (patrol, hover on a sine table, stationary hazard, spawner) driven by the room's foe list | each behaviour has a test that reads its position over N frames | L | Open (only the drone's patrol exists, inside `room.c`) |
| 5.2 | Hero vs foe: contact = `progress_hit()` (`HIT_TIME_PENALTY_S` and `INVULN_FRAMES`), `hero_knock()`, `sfx_crash()`, room no longer clean. No death, no respawn | test: a touch costs the penalty and starts the invulnerability | M | Done for the drone |
| 5.3 | Sprite budget in every scene | `tests/sprites.lua` measures the busiest scanline from OAM: race 6, crash burst 7, warp 8, room 4, map 2; add the worst room and the boss when they exist | S | Done for today's scenes |
| 5.4 | Moving platforms (carry Geeko) and mines, only when an era needs them (2010, 2020) | test: Geeko on a moving platform moves with it | M | Open |
| 5.5 | Guard vision cone | **cut by default** | - | Cut |

### M6. Era content

Repeat per era; 1990 first because 4.8 already built most of it.

| # | Step | Done when | Size | State |
|---|---|---|---|---|
| 6.1 | Era recipe: BG and sprite tiles in `build_chr.py`, palettes, room rectangles in `check_rooms.py`, foe list, story text, warp year; RACE background recoloured per era (optional) | bot playthrough completes the room; glyph and sprite checks pass | M per era | Open |
| 6.2 | 1990: add the guard (patrol) and floppy frisbees (projectile) to the drone | as above | M | Open |
| 6.3 | 2000 (Y2K bug, popup, paperclip; falling CRTs) | as above | M | Open |
| 6.4 | 2010 (laptops on platforms; lasers, sparking machine) | as above | M | Open |
| 6.5 | 2020 or 2054 (D2; phones, clouds, bots; moving platforms) | as above | M | Open |

### M7. Boss: The Mainframe

Design (arena map, agents, lanes, overload): section 8b. Nothing is built.

| # | Step | Done when | Size | State |
|---|---|---|---|---|
| 7.1 | Arena (8b.1): 3 tiers x 2 terminals on the room engine (a room without a gate), Mainframe head and eye as background art, 120-frame hacks, each terminal gives the time bonus | all 6 terminals hackable and reachable in a bot test | L | Open |
| 7.2 | Agents and lanes (8b.2): PINCHBOT, WHIRLY, TWINKLE; lane = (tier, side); fixed 16-entry spawn table, at most 3 on screen; a terminal closes its lane, both of a tier close the type; an agent hit costs the hit penalty | test: lanes close one per terminal (lane flags in RAM); a hit costs the penalty once | L | Open |
| 7.3 | Boss bar in the HUD (8b.3): one `#` per unhacked terminal | test reads the HUD row | S | Open |
| 7.4 | Overload (8b.3): 180 frames of flashes, eye cracks and a room-only shake through the IRQ array, then WIN with the time left and the finish bonus (`FINISH_POINTS_PER_S`) | test: hack 6, WIN reached | M | Open |

### M8. Polish and release

| # | Step | Done when | Size | State |
|---|---|---|---|---|
| 8.1 | Credits screen with the licence notices (nesdoug MIT, neslib public domain, FamiTone2, the CC0 track) | screen exists, glyph test | S | Open |
| 8.2 | Sound: era music (more songs, same export path), dedicated effects for the hack tick, a finished terminal, the gate, a hit | `smoke.lua` audio checks extended | M | Open |
| 8.3 | Title screen art, ending screen; set both cheats (`DEV_LEVEL_SELECT`, `DEV_INVULNERABLE`) to 0 | screenshots | M | Open |
| 8.4 | `README.md` and `AGENTS.md` match the finished game; real-hardware or flash-cart run if available | docs reviewed | S | Open |
| 8.5 | Optional: hi-score saved in battery PRG-RAM (`MAPPER_USE_BATTERY`; not tried) | survives an emulator restart | S | Open |

### Cut lines

* **A, after M2:** the race with the clock, warp, story text; no rooms. Reached except for the story screens.
* **B, vertical slice:** RACE, WARP, a 1990 room with a foe, result card, back to the RACE. **Reached as a draft** (room 1 with the drone); missing for a presentable B: era art, story screens, the sequence table, sounds.
* **C, after M5 + 6.3-6.4:** three eras.
* **D:** everything, with the boss.

Order of risk now: the first MMC3 code (1.1, 3.1, 3.2) and the era art are the unknowns; jump and collision feel is settled by the measured numbers and the bot, but nobody has played it by hand yet.

---

## 8a. Room design (detailed)

Design of the single-screen hacking rooms (steps M4, M5, M6). **Room 1 is built; rooms 2-4 are design only.** Everything marked *proposed* is a starting value. The maps are drafts: they are checked for reachability by `tools/check_rooms.py` (`python3 tools/check_rooms.py -v` prints them; it models the jump envelope below with the constants read from `src/geeko.h`; it is **not** the emulator), so a real recorded-input playthrough (steps 4.8, 6.x) is still the proof.

### 8a.1 Screen anatomy

* 32 x 30 tiles, one nametable, no scroll (scroll 0; on MMC3 the IRQ array would be `{0xFF}`). Rows 0-2 are the HUD (text in rows 1-2, as in the race). Play area rows 3-27. **Floor top = row 26**; rows 26-29 are solid fill (rows 28-29 can be cropped by a TV, keep nothing important there). Geeko standing on the floor: sprite top y = 26 x 8 - 16 = 192.
* Platform levels every 4 tiles (32 px): floor 26, then 22, 18, 14, 10. Platforms are 1 tile thick. The left and right screen edges are walls (x clamped in code, no tiles).
* **No pits** (nobody dies): the floor is continuous, falling just lands on a lower surface.

### 8a.2 Movement envelope (the numbers the layouts obey)

Simulated with the 4a constants in the same fixed-point order as `geeko.c` (`vel += gravity; y += vel`, impulse -0x4C0, gravity 0x38). The full-jump apex was **measured in the emulator** (49.2 px, `tests/room.lua`); the rest of the table is the simulation:

| Rise to the target (tiles) | Frames to land on it | Reach at 1 px/frame | Reach at 2 px/frame |
|---|---|---|---|
| 0 | 43 | 43 px | 86 px |
| 1 | 41 | 41 px | 82 px |
| 2 | 39 | 39 px | 78 px |
| 3 | 37 | 37 px | 74 px |
| 4 | 34 | 34 px | 68 px |
| 5 | 31 | 31 px | 62 px |
| 6 | 25 | 25 px | 50 px |

Apex of a full jump 49.2 px (6.1 tiles). Frames to full speed 22 (23.7 px), to a stop 32 (31.0 px).

**Layout rules** (conservative: 1 tile of margin, partial-speed jumps): rise at most 4 tiles (5 is the hard limit, 6 leaves 1 px: never); gap edge to edge at most 4 tiles for a rise of 0-1, 3 tiles for 2-4, 2 tiles for 5; drops any height with a gap of at most 6 tiles; at least 2 free tiles above every standing surface (Geeko is 2 tiles tall); a platform directly above another cannot be jumped through, reach it from a side.

### 8a.3 Tiles and data

* **BG tile classes** (the era banks of M3): `0x00-0x7F` common (blank, hack-bar tiles `0x01-0x09`, font); `0x80-0xBF` and `0xE0-0xFF` era decor (not solid); **`0xC0-0xDF` era solid** (floor, platform, rack, with edge variants). `room_solid(t) = (t & 0xE0) == 0xC0`. About 32 solid and 96 decor tiles per era.
* **Terminal** = 2 x 2 tiles with two states (blue screen, green `>_`) = 8 era tiles. **Progress** is a 4-tile bar one row above the terminal, drawn with the 9 common tiles `0x01-0x09` (empty to full), so no per-era progress art. Hack zone = the terminal's two columns plus one column on each side, standing on that surface.
* **Gate** = 2 x 3 tiles, closed and open (12 era tiles), on a surface, opens when every terminal is green.
* **Room data** in PRG (as built for room 1): a list of solid rectangles, terminals, gate, start and foe list, generated into `assets/rooms.h`; `room_load()` draws the rectangles and the attribute table (terminals palette 1, gate palette 2; terminals and the gate need even tile rows and columns for their palette) and `room_solid()` reads the same list. The raw 960-byte nametable of the first design was dropped (plan step 4.1). Hack frames come from `tuning.h`.
* **Authoring**: rectangles (row range, column range) plus object lists in `tools/check_rooms.py` (the reachability check, `-v` prints the maps); `tools/build_rooms.py` turns the playable rooms into `assets/rooms.h`. The rectangles of the maps below are that input. Only rooms with a gate and drone-only foes are playable so far.

### 8a.4 Foes: four behaviours, parameters per room

All 16 x 16, at most 4 per room and at most 3 on the same rows (Geeko 2 sprites + 3 x 2 = 8 per scanline). **Built: the drone's PATROL** (speed 0.5 px/frame, between two columns, in `room.c`); the other behaviours are design.

| Behaviour | Parameters | Used by |
|---|---|---|
| PATROL | platform range (columns), speed 0.5 px/frame (slow) or 0.75 (fast); turns at the ends | drone, guard, Y2K bug, padlock |
| HOVER | centre, amplitude, period (64-entry sine table), horizontal or vertical | popup, hook, bots |
| STATIONARY | period, on-frames, 20 frames of blinking before it turns on | laser, sparking machine |
| SPAWNER | interval, speed, 30 frames of shadow before a falling object | falling CRT, boss agents |

Contact (any behaviour; built for the drone): `progress_hit()` = `clock_sub(HIT_TIME_PENALTY_S)` and `INVULN_FRAMES` of invulnerability, `hero_knock()`, `sfx_crash()`; the room is then no longer clean. Foe movement uses tables and counters, no random numbers, so recorded-input tests stay reproducible.

### 8a.5 The four rooms

Legend: `#` solid, `TT` terminal (2 x 2), `GG` gate (2 x 3), `S` Geeko's start, letters = where a foe starts (d drone, b bug, p popup, c CRT spawner, l laser, s sparking machine, k padlock, h hook, o bot). Column numbers on top. The route is the intended order, not the only one. Clean time is my **guess**, to be replaced by the measured playthrough.

**Room 1: 1990 UNIX Labs** (red brick, CRT glow). **Built.** 2 terminals, hack 60 frames. Teaches: jump a ledge, hack, avoid one foe.
Rectangles: floor `26-29, 0-31`; platforms `(22, 3-9)`, `(18, 12-21)`, `(22, 24-29)`. Terminals `(18, 16)`, `(22, 26)`. Gate floor cols 0-1. Start floor col 10.
```
    01234567890123456789012345678901
15  ................................
16  ................TT..............
17  ................TT..............
18  ............##########..........
19  ................................
20  ..........................TT....
21  ..........................TT....
22  ...#######..............######..
23  GG..............................
24  GG............d.................
25  GG........S.....................
26  ################################
27  ################################
28  ################################
29  ################################
```
Route: right along the floor (jump the drone), onto the right shelf, terminal 2, up-left to the top shelf, terminal 1, drop left, down to the floor, gate.
Foes: drone, PATROL on the floor, columns 14-22, slow, hovering at head height (rows 23-24, 8 px above the floor: jump over it). Clean time about 25 s (guess); the test bot, which takes the shelves and never meets the drone, needs 10.5 s.

**Room 2: 2000 Dot-com farm** (steel racks, blue LEDs). 3 terminals, hack 75 frames. Teaches: hurdles, timing a hazard.
Rectangles: floor; racks `(23-25, 9-10)`, `(23-25, 24-25)` (3 high: hurdles); platforms `(22, 1-6)`, `(18, 8-15)`, `(22, 17-22)`, `(18, 26-30)`. Terminals `(22, 3)`, `(18, 11)`, `(18, 28)`. Gate floor cols 29-30. Start floor col 7.
```
    01234567890123456789012345678901
 3  ................................
 4  .............c.....c............
 5  ................................
 6  ................................
 7  ................................
 8  ................................
 9  ................................
10  ................................
11  ................................
12  ................................
13  ................................
14  ................................
15  ................................
16  ...........TT...............TT..
17  ...........TT...............TT..
18  ........########..........#####.
19  ................................
20  ...TT..p........................
21  ...TT...........................
22  .######..........######.........
23  .........##.............##...GG.
24  .........##.............##...GG.
25  .......S.##.b...........##...GG.
26  ################################
27  ################################
28  ################################
29  ################################
```
Route: left under the first shelf, up to it (terminal 1), up-right to the middle shelf (terminal 2), drop to the third shelf, up to the right shelf (terminal 3), drop, gate.
Foes: Y2K bug PATROL on the floor, columns 12-23, fast; popup HOVER vertical in the gap between the first two shelves (column 7, centre row 20, amplitude 2 tiles); two falling CRTs from the ceiling at columns 13 and 19 (SPAWNER: shadow 30 frames, period 120). Clean time about 35 s.

**Room 3: 2010 Cloud cleanroom** (white panels, cyan fibre). 3 terminals, hack 90 frames. Teaches: laser timing across the floor, a hazard on a hack spot.
Rectangles: floor; platforms `(22, 24-29)`, `(18, 17-22)`, `(14, 9-14)`. Terminals `(22, 26)`, `(18, 19)`, `(14, 11)`. Gate floor cols 0-1. Start floor col 4 (next to the gate: the route loops back).
```
    01234567890123456789012345678901
 3  ................................
 4  ............s...................
 5  ................................
 6  ................................
 7  ................................
 8  ................................
 9  ................................
10  ................................
11  ................................
12  ...........TT...................
13  ...........TT...................
14  .........######.................
15  ................................
16  ...................TT...........
17  ...................TT...........
18  .................######.........
19  ................................
20  ..........................TT....
21  ..........................TT....
22  ........................######..
23  GG..............................
24  GG..............................
25  GG..S.k.....l...................
26  ################################
27  ################################
28  ################################
29  ################################
```
Route: right along the floor through the lasers, up the three platforms (terminals 1, 2, 3), drop left to the floor, gate.
Foes: laser tripwire STATIONARY across the floor, columns 12-18, 60 frames on and 60 off; sparking ceiling machine at column 12 dropping sparks on the third platform's hack zone (STATIONARY, period 120, 40 on): a spark knocks Geeko out of the zone and resets the hack; padlock PATROL on the floor, columns 5-9, slow. Clean time about 40 s.

**Room 4: 2020 AI edge** (dark chassis, neon green). 4 terminals, hack 105 frames. Teaches: a moving platform, a floor terminal between hazards.
Rectangles: floor; platforms `(22, 2-7)`, `(22, 24-29)`, `(14, 14-19)`; the platform at `(18, 10-17)` is **moving**: 4 tiles wide, sliding between columns 10 and 17 (the map shows its whole travel; the check treats it as fixed). Terminals `(22, 4)`, `(22, 26)`, `(14, 16)`, floor `(26, 15)`. Gate floor cols 30-31. Start floor col 0.
```
    01234567890123456789012345678901
11  ................................
12  ................TT..............
13  ................TT..............
14  ..............######............
15  ................................
16  ................................
17  ................................
18  ..........########..............
19  ................................
20  ....TT.o..............o...TT....
21  ....TT....................TT....
22  ..######............h...######..
23  ..............................GG
24  ...............TT.............GG
25  S..............TT.............GG
26  ################################
27  ################################
28  ################################
29  ################################
```
Route: up to the left shelf (terminal 1), across on the moving platform, up to the top island (terminal 3), drop to the floor terminal (4), right, up the right shelf (terminal 2), drop, gate.
Foes: phishing hook HOVER vertical at column 20, centre row 22, amplitude 3 tiles (it blocks the floor path between terminal 4 and the right shelf); two bots HOVER horizontal along row 20, columns 6-14 (over terminal 1) and 16-24 (over the approach to the right shelf). Moving platform needs "carry Geeko" (step 5.4). Clean time about 50 s.

### 8a.6 Time economy (one 10-minute clock, `tuning.h`)

Per epoch, with a 60-second race (decided) and the guessed room times:

| Epoch | Race | Room | Spent | Terminals | Bonus (10 s each) | Net |
|---|---|---|---|---|---|---|
| 1990 | 60 s | 25 s | 85 s | 2 | +20 s | -65 s |
| 2000 | 60 s | 35 s | 95 s | 3 | +30 s | -65 s |
| 2010 | 60 s | 40 s | 100 s | 3 | +30 s | -70 s |
| 2020 | 60 s | 50 s | 110 s | 4 | +40 s | -70 s |
| Boss | - | 60 s | 60 s | 6 | +60 s | 0 s |
| **Total** | | | **450 s** | 18 | **+180 s** | **-270 s** |

A flawless run spends 450 s and earns 180 s: **net 4:30 of the 10:00, ending with 5:30 left**. A race crash costs 5 s plus about 9 s of slowdown (about 14 s), a contact 5 s: about 23 race crashes or 66 contacts use the spare 5:30. A first-time player at twice my guessed room and boss times (+150 s rooms, +60 s boss) nets 8:00 and ends with about 2:00 left, so sloppy play can run out the clock. These are guesses; the playthrough tests replace them with measured times.

### 8a.7 Score (decided 2026-10-19, numbers in `tuning.h`)

Score measures what the clock does not: flow, speed, no hits.

| Source | Rule | Built |
|---|---|---|
| Speed | every 4th frame of a race: mph / 20 points (20 MPH gives 1, 88 MPH gives 4) | yes |
| Dodges | an obstacle that passes Geeko untouched: 10 x multiplier; the multiplier is 1 + (dodges in a row) / 3, at most 5; a hit puts it back to 1; a beep when it rises | yes |
| Terminals | 100 per hacked terminal | yes |
| Clean room | 250 for a room finished without being hit | yes |
| Finish | seconds left on the clock x 10, only for winning | no (needs the boss and the WIN state) |
| Par-time bonus per room, hidden pickups, hi-score in battery RAM | | no |

* **Measured (smoke test):** a clean 60 s race = 63 obstacles passed, 4,907 points.
* **Flawless game model** (the smoke test computes it from the measured race and the plan's guessed room times): 4 races 19,628 + 18 terminals 1,800 + 5 clean rooms 1,250 + finish 3,300 (330 s left) = **25,978**, tuned to the 26,000 target; the test fails if it drifts by more than 500 (the race part moves by about 100 per obstacle, so it depends on the random seed).
* **Ranks** by final score: NEWBIE below 5,000, SCRIPT KIDDIE below 13,000, SYSADMIN below 21,000, ROOT from 21,000. Shown on the game-over screen and on the result card.
* **Result card** after each room: race time, terminals, clean-room bonus or none, score, rank.
* `score` is a 16-bit integer: it saturates at 65,535 (`score_add()`); a flawless game is about 40% of that.

### 8a.8 Backgrounds and the time map (built 2026-10-19)

**Backgrounds** (decided): the race has a **blue sky** (`C_SKY_BLUE` 0x11 in `palettes.h`: the first colour of every race palette is the screen's background colour); rooms are **black** (`room_bg_palette`); the time map is black too. The test checks the background colour (palette RAM entry 0): `0x11` in the race, `0x0f` in a room and on the map.

**The time map** (`map.c`, `gfx.c`; shown after the title and after every result card, before the race): five nodes on a line at columns 4, 10, 16, 22, 28 (2x2 tiles, row 10) for 1990, 2000, 2010, 2020 and the boss (2054), a dotted path between them, the years under the nodes, "NEXT STOP" and the era's name (UNIX LABS, DOT-COM FARM, CLOUD CLEANROOM, AI EDGE, THE MAINFRAME) below, PRESS A TO GO. Eras already played are green with a tick, the next ones blue, the boss is a red eye. **Geeko (the riding sprite) blinks over the next node** (8 frames on, 8 off). The HUD stays on top (the clock is stopped). It stays `MAP_FRAMES` (180 = 3 s); A or Start skips it after `MAP_SKIP_FRAMES` (20), so the press that started the game cannot skip it. **Level select** (`DEV_LEVEL_SELECT`): LEFT/RIGHT move the selection (the nodes recolour, the caption changes, Geeko moves, the timeout stops), A starts that era's race, SELECT its room, the boss node ends the game; the map shows the keys while the cheat is on. The boss's map ends the game with TO BE CONTINUED, score and rank (the boss is not built). The era counter is `epoch` in `progress.c` (0-3 the eras, 4 the boss); the HUD year, the warp's year roll and the room's hack time follow it.

Terminals and node icons are 2x2 tiles on even rows and columns, so each can have its own palette (the attribute table has one palette per 2x2 tiles); `gfx_attr_set()` sets it.

---

## 8b. Boss design (detailed): THE MAINFRAME

Design of the final arena (step M7). **Not built.** Same room engine, same rules as 8a, drafts as in 8a.

### 8b.1 Arena

Rectangles: floor; bottom tier `(22, 1-7)` and `(22, 24-30)`; middle tier `(18, 8-13)` and `(18, 18-23)`; top tier `(14, 3-8)` and `(14, 23-28)`. Terminals: bottom `(22, 3)` and `(22, 27)`, middle `(18, 10)` and `(18, 20)`, top `(14, 5)` and `(14, 25)`. Start floor col 15. No gate: the sixth terminal triggers the overload. Hack 120 frames each. The Mainframe itself is background art above row 12: a monolith head (about cols 12-19, rows 3-11, decor tiles, not solid) with a 4 x 4 tile eye; the centre of the arena is open, so Geeko can cross on the floor.
```
    01234567890123456789012345678901
11  ................................
12  .....TT..................TT.....
13  .....TT..................TT.....
14  ...######..............######...
15  ................................
16  ..........TT........TT..........
17  ..........TT........TT..........
18  ........######....######........
19  ................................
20  ...TT......................TT...
21  ...TT......................TT...
22  .#######................#######.
23  ................................
24  ................................
25  ...............S................
26  ################################
27  ################################
28  ################################
29  ################################
```
Route per side: floor, up the bottom tier, middle tier, top tier; the sides are linked across the floor. Minimum hacking time 6 x 2 s = 12 s; clean boss time about 60 s (guess).

### 8b.2 Agents and lanes

An agent type belongs to a tier; a **lane** is (tier, side): agents of that tier leave the eye towards that side at head height of that tier. Both of the older rules hold: **a hacked terminal closes its own lane** (tier, side); **both terminals of a tier close the agent type for good**; every terminal also takes 1/6 off the boss bar.

| Agent (renamed) | Tier / terminals | Movement (all *proposed*) |
|---|---|---|
| PINCHBOT (orange crab) | top: `(14, 5)`, `(14, 25)` | fast 1.5 px/frame outward at tier head height (sprite y 96), vertical jitter of at most 8 px, changes every 16 frames |
| WHIRLY (teal swirl) | middle: `(18, 10)`, `(18, 20)` | 0.75 px/frame outward on a sine, amplitude 16 px, period 64 frames, centre y 128 |
| TWINKLE (cyan star) | bottom: `(22, 3)`, `(22, 27)` | 1 px/frame outward along the floor, bouncing 24 px high every 40 frames (floor y 192) |

* **Spawner:** every `SPAWN_FRAMES` (proposed 90, minus 10 per hacked terminal, so the boss gets faster as it gets weaker) take the next open lane from a fixed 16-entry table (no random numbers: reproducible tests), skip it if 3 agents are on screen. The agent appears at the screen centre with 20 frames of flash, then flies out; leaving the screen frees its slot.
* **Sprites:** 3 agents x 2 + Geeko 2 = 8 per scanline at most (the measured limit of the race holds here too).
* **Contact:** same as a room hit (`clock_sub(HIT_TIME_PENALTY_S)`, knockback, invulnerability); a hit also leaves the hack zone, which resets the hack.
* **Terminals:** each one finished gives `clock_add(HACK_TIME_BONUS_S)`.

### 8b.3 HUD and finish

* HUD row 1: `THE MAINFRAME` followed by one `#` per unhacked terminal; row 2: `TIME mm:ss` and score.
* **Overload** (sixth terminal): 180 frames. Agents vanish, the eye cracks in 3 tile stages, the palette flashes every 8 frames, and the room shakes while the HUD stays still: the shake is the IRQ array `{22, 0xF5, shake_x, 0xFF}` (an MMC3 gain: the sprite-0 split could not shake only the room). Nametable B's left and right edge columns must be filled so the shake shows no garbage. Then the `WIN` row: ending story screen and the time left.
* **Background colour (decided 2026-10-19):** a **deep violet** (`0x03`) rather than black: it makes the red eye, the orange agents and the green terminals stand out, and it is clearly not the black of the other rooms. Alternatives: a dark crimson (`0x06`, danger, but the red eye blends in) or the room black with red flashes. The overload flashes the whole palette (`pal_bright`) as the warp does.
* **Tile budget (estimate, tight):** head about 12 + eye 2 frames x 16 = 32 + terminal 8 + cracks about 12 = 64 decor tiles, which is why the decor range is 96 tiles (`0x80-0xBF` and `0xE0-0xFF`).

### 8b.4 Tests

Recorded-input playthrough (fixed spawn table): hack all six, reach `WIN`; lanes close one per terminal (read the lane flags from RAM); an agent hit lowers the clock by the penalty once; the boss bar in the HUD row equals the unhacked count; at most 8 sprites per scanline in the worst scene.

---

## 9. Decisions

Decided (user, 2026-10-19):
* **No death, no lives, no health.** A race crash slows Geeko down (reaching 88 MPH takes longer) and costs fixed time; a foe contact costs fixed time; a hacked terminal adds time; the only game over is the clock reaching 0.
* **One clock for the whole game, 10 minutes** (first said 26 minutes, cut to 10 so the clock fits a 16-bit frame counter); a game over restarts the whole game with a full clock; a race takes 60 s.
* **Game-over screen** shows GAME OVER, score, rank and PRESS A TO RESTART.
* **Score** (8a.7): points scale with speed, dodge streak multiplier, hacked terminals, clean rooms, a finish bonus; ranks; a flawless game is tuned to about 26,000.
* **Balance numbers are plain `#define`s in `src/tuning.h`.**
* **A time map** is shown between levels with Geeko blinking over the next era (8a.8).
* **Two test cheats**, level select and invulnerability, each switched by a `#define` before the build.
* **Backgrounds:** blue sky for the race, black for the rooms, deep violet (`0x03`) for the boss arena (built with the boss, step 7.1: a palette like `room_bg_palette` with that first colour).
* **One Geeko body** (`geeko.c`) serves every level; the code is kept free of duplicated parts (shared helpers: `geeko.c`, `gfx.c`, the test `lib.lua`).
* **Mapper: MMC3** (section 7a).

Decided earlier:
* **RACE + HACK** both stay in the game; the race is the way into each epoch (2026-10-12).
* **No combat**, no tongue; **hack = hold B**; **no tail-hang**: rooms use Mario-style jumping on the hoverboard with inertia on start and stop (2026-10-12).
* **Geeko in rooms:** the 16x16 sprite (2x2 tiles) is used in the race and in rooms (2026-10-16).
* **Music:** FamiTone2, with FamiStudio as the converter (2026-10-16).
* **Licence of this repo:** GPL-3.0 (`LICENSE`, 2026-10-15), see section 10.

Still open:
| # | Question | Default if not answered | Needed by |
|---|---|---|---|
| D2 | 4th epoch: 2020 (the brief) or 2054 | 2020, with the Mainframe as the 2054 AI era | 6.5 |
| D3 | Story text: uppercase only, or add lowercase | uppercase only | 2.2 |
| D4 | Rooms per epoch (10 minutes fits one; more rooms need a longer clock, and the 16-bit counter stops at 18:12) | 1 | 6.1 |
| D5 | Branding: the openSUSE logo and AI agents named after real products in the concept art. Proposed renames: **PINCHBOT** (orange blocky crab), **WHIRLY** (teal swirl), **TWINKLE** (cyan star), redrawn rather than copied (`concept-art/PROMPTS.md`). Check the openSUSE logo guidelines | proposed names | 7.2 |
| D9 | Time economy amounts (`tuning.h`: penalty 5 s, bonus 10 s, cap 600 s). A flawless run needs about 4:30 of the 10:00 (8a.6); a first-time player at twice the guessed room and boss times is near the limit | keep the proposed amounts | tuning |
| D10 | Hit knockback and invulnerability (10 frames of knockback, `INVULN_FRAMES` 60) are my values | keep | tuning |

---

## 10. Licences of the neighbouring projects

Checked 2026-10-12 from each project's `LICENSE` / README in `nesdev/` (siblings of this repo). Licence files cover the repo's *code*; art, music and levels can carry their own terms, noted where the README says so.

| Project (upstream) | Licence | Reusable here? |
|---|---|---|
| `nesdoug-llvm` (mysterymath's llvm-mos port of Doug Fraker's nesdoug tutorials) | MIT, Copyright 2018 Doug Fraker | Yes, with the notice kept. **Our ROM already links `nesdoug`** (and `neslib`) |
| `neslib` (Shiru), as shipped in the SDK | Header says "consider it Public Domain" | Yes. Also linked already |
| `file-fixers` (wendelscardua; computer-themed RPG, Game Off 2021) | MIT, 2021 Wendel Scardua | Yes. Closest in theme, MMC3, but uses cc65 and FamiTone 5 |
| `mbh-A53-witchnwiz` (mhughson) | MIT, 2022 Matthew Hughson. Credits: Shiru (neslib, famitone), nesdoug, norill (donut integration, mapper 28), JRoatch (donut compression), each with their own terms | Yes for the game code; check the credited libraries separately |
| `famidash` (tfdsoft; a Geometry Dash fan game) | MIT, 2023-2024 TFDSoft | Code yes. It is a fan game: names and levels are third-party IP, and the music/graphics are by many contributors with no separate licence stated. Do not copy content |
| `nes-starter-kit` (cppchriscpp) | MIT text (copyright holder line left blank). README: art and music "technically public domain (CC0)", asks for credit | Code yes; assets per README, credit them |
| `from-below` (mhughson/mbh-firstnes) | `game/License.txt` is **nesdoug's** MIT; the repo has no licence for the game itself | **No**: unlicensed game code/art/music means all rights reserved. Ideas only |
| `MK1_NES` (Mojon Twins, MT Engine MK1) | **LGPL-3.0**; README: game assets in the testers and examples are donationware | Linking is fine; copying engine code carries LGPL terms. Do not copy assets |

What this means for this repo (`LICENSE` is GPL-3.0 since 2026-10-15; the bullets below are the reasoning that led there):
* Nothing here is *forced* on us as long as we copy no code or assets. We already ship MIT / public-domain library code (nesdoug, neslib, FamiTone2) in the ROM: keep a credits file with their notices (step 8.1; the README credits the music track).
* Copying nothing from other projects, any licence works. MIT would match most of the neighbours. **Chosen: GPL-3.0** (`LICENSE`).
* The MIT/CC0 projects (`file-fixers`, `witchnwiz`, `nes-starter-kit`, `nesdoug-llvm`) are safe sources if we want a reference implementation to copy from; `from-below` and `famidash` content are not.
