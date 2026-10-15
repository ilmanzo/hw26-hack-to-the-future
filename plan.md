# Hack to the Future (NES) - Game Concept & Implementation Plan

Geeko (the openSUSE chameleon) races on an anti-gravity hoverboard until he hits **88 MPH**, travels through time, and **hacks the future**: in each IT era he must hack every terminal and install openSUSE, avoiding era-themed foes. Final boss: **The Mainframe**.

**Two halves, one game** (decision 2026-10-12):
* **RACE**: the NES version's own endless runner (already built): jump and duck on a road while the speed climbs from 20 to 88 MPH.
* **HACK**: ported from the MSX sibling (`../../msxdev/hack-to-the-future/`: `plan.md`, `README.md`, `GEMINI.md`): stealth rooms, terminal hacking, tail-hang, the six-terminal Mainframe boss, story screens. Section 1a lists what is ported and how.

Numbers marked *proposed* are starting values to tune, not decisions.

---

## 0. Status (checked 2026-10-12)

Verified by reading the code, `make test` (headless Mesen2, see AGENTS.md) and screenshots.

| Piece | State |
|---|---|
| Endless runner (road, jump/duck, 3 obstacle types, score, 20 -> 88 MPH) | Works, smoke-tested |
| Geeko sprite (24x24 riding, 24x16 ducking, from the openSUSE logo) | Done |
| HUD with sprite-0 split | Works (split bug fixed, regression-tested) |
| Time warp at 88 MPH | **Only a sound** (known issue 3) |
| Epoch screens, terminals, hacking, era foes, boss | Not started |

### Known issues (not fixed)
1. **Score HUD shows score x 10.** `hud_update()` (`hud.c`) writes the 5 digits at column 15, but the static text `SCR:00000` has its digits at 16-20. The colon is overwritten and a stale `0` stays at column 20 (code + screenshot: `SCR000270`). Fix: column 16.
2. **A standing player is not hit by a drone.** Drone hitbox y138-147, standing hitbox starts y150 (confirmed in the emulator, `XFAIL` in `tests/smoke.lua`). "Duck under drones" currently does nothing.
3. **The 88 MPH warp is only a sound.** `warp_flash_timer` is set to 30 in `progress.c` and never read or decremented: no flash, no transition.
4. ~~**`README.md` is out of date.**~~ **Fixed 2026-10-15:** rewritten (current state, controls, build/test, licence).
5. **8 sprites per scanline not measured.** Geeko is 3 tiles wide now; 2 ground obstacles + Geeko + fire trail can exceed 8. Draw order protects obstacles (trail is last), but nobody has looked at it in the emulator.
6. `Makefile` hardcodes `CC` to a path that does not exist on this machine (builds go through the `Containerfile`).
7. The font has no `'` or `,`: the title strings `GEEKO MCFLY '85` and `WHERE WE GO, NO ROADS!` show blanks there.

---

## 1. The loop

```
        +--------------------------- repeat for each epoch ----------------------------+
        |                                                                              |
TITLE -> STORY -> RACE (road, jump/duck, 20 -> 88 MPH) -> WARP (flash, year rolls) -> ROOM (hack terminals)
                                                                                         |
                                       all terminals green -> exit gate opens -> reach it -> "INSTALL COMPLETE"
                                                                                         |
                                                              last epoch done?  no -> STORY -> RACE ...
                                                                                yes -> STORY -> BOSS: THE MAINFRAME
                                                                                              |
                                                                                              v
                                                                                         ENDING (WIN)
```

* **RACE is the way into the future.** Reaching 88 MPH is what triggers the warp; the runner also supplies the score. It is already built and tested.
* **ROOM is the MSX-style hack level**: one single screen (no scrolling) with platforms, terminals, foes, hazards and an exit. Geeko rides the hoverboard Mario-style: run with inertia, jump (section 4a). No weapons.
* The game is driven by a **sequence table** (like the MSX `InitLevels()`): a list of `STORY`, `RACE`, `ROOM(n)`, `BOSS` entries; main.c steps through it. Adding content = adding rows.
* Dying costs a life (3 lives, *proposed*). Game over restarts the current epoch's RACE.
* Rooms per epoch: 1 for the first playable version; the room format allows several (the MSX game has 8 per world).

---

## 1a. What is ported from the MSX game

Read from the MSX repo on 2026-10-12 (`README.md`, `GEMINI.md`, `plan.md`, `src/gameplay/level_defs.h`, `src/main.c`). The MSX game has no race: it is a pure stealth puzzle-platformer.

| MSX idea | MSX source / state | NES adaptation | Cost |
|---|---|---|---|
| **Premise**: 2054, a closed-source Megacorp's AI mainframe purges open-source OSes; Doc the UNIX hacker built the hoverboard; Geeko warps through four computing eras | README | Same premise on the story screens | Text only |
| **Four worlds = four eras**: 1990s UNIX Labs & BBS (red brick, CRT), 2000s dot-com server farm (cobalt racks, blue LEDs, DRM firewalls), 2010s cloud hyperscalers (white cleanroom, cyan fibre), 2054 AI Citadel (obsidian, neon green) | README | Four epochs, see section 3 (moods reused, hardware per your brief) | One CHR bank each |
| **Stealth, zero combat** | README / GEMINI.md | No weapons: foes are avoided, never killed. The tongue stays out (except maybe as a visual) | Simplifies foes and boss |
| **Terminal hack**: stand in front and **hold a button** for 45 frames (~0.75 s); tick sound while channelling; terminal turns green `>_` | README; plan.md 6 | **Decided: hold B** (A = jump); 60-120 frames; progress drawn on the terminal | Low |
| **Exit gate**: all terminals hacked unlocks the exit; reach it to finish the level | README (design). *As built the MSX has one terminal per level (`key_x/key_y`), no counter* | Count terminals left; exit gate opens at 0; reaching it ends the room | Low |
| **Tail-hang**: `UP` clings to a platform underside or ceiling rail to dodge patrols, `DOWN` drops | plan.md 5.2 (as built: peeks 2 tiles above the head, no raycast) | **Dropped (decision 2026-10-12).** Replaced by Mario-style jumping with inertia on the hoverboard, section 4a. No ceilings, no hang states | None |
| **Level data**: `struct Level` = start, exit, terminal, crystal, platforms[], mines[], enemies[], layout, name (max 4 platforms, 6 mines, 4 enemies) | `level_defs.h` | `struct Room` with the same shape; layout = nametable + 1-bit-per-tile collision map | Medium |
| **Hazards**: linear moving platforms (min/max), static mines, patrol enemies in 4 types: type 0 slow (2/8 px per frame), others 4/8; type 2 stationary energy field, type 3 fires a projectile | `level_defs.h`, `gameplay_engine.c` | Three behaviours: patrol, stationary hazard (field / laser / spark), projectile. Era re-skins. Max ~4 per room (8-sprites-per-scanline budget) | Medium |
| **Mainframe boss**: 6 terminals in 3 tiers (left/right), agents Clawd / ChatGPT / Gemini; hacking a pair disables that agent type; all 6 = overload | plan.md 9. *As built the boss is broken: wrong platform data, only one agent at a time, undifferentiated behaviour* | Section 5. Do not port the bugs | High |
| **Story screens**: intro, mid and end of each world, pre-boss, pre-fight, final, true ending (about 15) | `cutscene_scripts.c` | Text on a nametable (`vram_write`), about 6-8 screens: intro, one per epoch, pre-boss, ending | Low (needs a lowercase/punctuation font) |
| **Sequence table** of `SEQ_LEVEL` / `SEQ_CUTSCENE` / `SEQ_BOSS` | `InitLevels()` in `main.c` | Same idea plus `RACE`, `WARP` | Low, makes content cheap |
| **Music**: PT3 tracker driver + ayFX effects | `audio_manager.c` | NES has raw APU pokes only; a sound engine is a later decision | Medium |
| **Asset pipeline**: `Toolkit/gen_*.py` ASCII art -> generated headers | Toolkit | Already the same approach (`tools/build_chr.py`) | None |
| Not ported | 32 levels + 3 secret levels (vortex detour), key/stomp mechanics from the legacy game, MSX sprite layering | - | - |

**Licence and provenance (read before copying anything).** The MSX project says its assets are "derived from Chrono Runner" and are **CC BY-NC-SA 4.0** (Associazione MSX Italia); its code is **GPL-3.0**. Porting *ideas* is free. Copying MSX code, level data, art or text into this repo brings those licences (attribution, non-commercial, share-alike, GPL), and the level maps may carry Chrono Runner's own provenance. The other NES projects next to this one are checked in section 10. This repo has no LICENSE file yet. *(Open decision 8.)*

---

## 2. Concept art (`concept-art/`)

Eleven images, named by content (renamed from `Gemini_Generated_Image_*` on 2026-10-15). The image files are not in the repo (`*.jpeg` is gitignored); they are linked from an external site. Four were inspected on 2026-10-12 (they existed as low-res `17917…jpg` plus a 2752x1536 copy; the low-res copies have since been removed) and are catalogued below. The other seven (renders from `concept-art/PROMPTS.md` and NES-style mock-ups, `nes-*` and `illustration-race-*` / `illustration-1990-*`) are identified by filename only.

| File | Shows | Design takeaways |
|---|---|---|
| `msx-box-cartridge-manual-photo.jpeg` (a photo mock-up of box + cartridge + manual; the flat cover art was `1791699937579.jpg`, now removed) | Box art "ゲーコング" by **GEEKO SOFT**. Geeko with goggles, thumbs up, spiral tail, sitting on a CRT between tower PCs; green binary rain over server racks; orange diagonal banner. **MSX/MSX2-branded**, not NES. | Tone and palette (black, openSUSE green, orange, CRT cyan glow). A NES box/cart label would need a re-brand. Geeko wears goggles; ours does not (cheap to add) |
| `illustration-server-room-tongue-hack-guard-drone.jpeg` | **1990 stage.** Two-tier steel catwalk with railings. Geeko crouches on the upper tier and hacks a beige tower + CRT showing a green `>_ openSUSE` prompt, with cyan particles. A server at the right shows a **blue screen**. Below: a hovering **security drone** and an **armoured guard**, each with a red **vision cone**. HUD: portrait + health bar, `LIVES x3`, score, inventory slots (tower/terminal/etc. with counts), tool slots (wrench, hourglass) | Unhacked = blue screen, hacked = green terminal (cheap, readable state change). Vision cones imply stealth: foes are dangerous when you are in the cone. Two tiers fit one NES screen |
| `illustration-tail-hang-laser-grid-laptop.jpeg` | **2010 stage.** Geeko **hangs by its tail** from an overhead pipe and hacks a **laptop** on a hovering platform (openSUSE logo on screen). Hazards: red **laser tripwires** across the floor, a sparking ceiling machine. HUD: health + energy bars, lives, score, wrench + laptop slots, openSUSE badge | The ceiling hang is not ported (jumping instead). Hazards that are not creatures (lasers, sparks) are cheap to implement and era-flavoured |
| `illustration-mainframe-boss-tongue-vs-crab.jpeg` | **Boss: THE MAINFRAME.** A monolith with a huge red slit-pupil eye, a teal swirl emblem, red circuit lines, smoke, flanked by server racks. Geeko leaps along a catwalk; its tongue is a magenta lightning bolt hitting an **orange pixel crab** (AI agent, shown glitching into green code); a second crab hovers; a cyan star sparkle. HUD: boss bar "THE MAINFRAME", player health + energy, lives, score | Boss = one static, big, readable sprite group + projectiles. The boss bar can be the count of unhacked terminals. The crab matches the "Clawd" agent of the MSX plan (likely; the art does not say) |

Conflicts between the art and this plan:
* The art hacks with the **tongue** (and uses it as a weapon on the boss). The plan follows the MSX design: **no weapons**, hack by standing in a zone and holding B. The tongue is not part of the sprite any more.
* The art is a side-scrolling platformer with health bars; the plan keeps 1-hit death + lives (simplest); a health bar is optional.

---

## 3. Epochs (all values *proposed*)

| # | Year | Terminal look | Terminals | Hack time | Foes (concept art in **bold**) | Hazards |
|---|---|---|---|---|---|---|
| 1 | 1990 | beige tower + CRT | 2 | 60 frames | **security drone**, **armoured guard with flashlight cone**, floppy disk frisbees | loose cables |
| 2 | 2000 | beige desktop + flat LCD | 3 | 75 | Y2K bug, popup windows, a paperclip-style helper | falling CRTs |
| 3 | 2010 | laptops (on hovering platforms) | 3 | 90 | malware padlock, "updating..." hourglass blob | **laser tripwires**, **sparking ceiling machine** |
| 4 | 2020 | phones / tablets / cloud racks | 4 | 105 | phishing hook, bot swarm | moving platforms |
| B | the AI era | the Mainframe's 6 terminals | 6 | 120 | The Mainframe + **crab AI agents** | - |

* Moods reused from the MSX worlds: 1990 UNIX labs / BBS (warm red brick, CRT glow); 2000 dot-com server farm (steel racks, blue LEDs, firewalls); 2010 cloud (white cleanroom panels, cyan fibre); 2020 / the boss: AI Citadel (dark chassis, neon green traces). The MSX 4th world is the year 2054; here the 4th epoch is 2020 per the brief and the Mainframe is the 2054 AI era.
* Era entry: warp flash, then a year counter rolls to the era (1985 -> 1990, ...), then the screen loads.
* Foes are era-flavoured re-skins of 4 behaviours, so code stays small: **patrol** (walks a platform), **hover** (sine path), **stationary hazard** (laser, spark, energy field), **projectile** (MSX enemy type 3). New foe = new tiles, not new code.

---

## 4. Hacking mechanic

* Stand in the terminal's front zone and **hold B** (MSX: hold Space / trigger A). A progress meter fills while held. MSX default is 45 frames (~0.75 s); NES proposed 60-120 by epoch (section 3). **Decided: hold B** (your first brief said just stand; holding avoids hacking by accident when sliding past a terminal).
* Progress is drawn on the terminal itself: blue screen -> tile frames filling -> green `>_ openSUSE`. Four background tile frames per terminal type. A tick sound plays every 8 frames while channelling (MSX does the same).
* Releasing B or leaving the zone resets progress (*proposed*). MSX design also aborts on damage (as built it only aborts on release or leaving); on NES a hit costs a life anyway.
* When every terminal in the room is green, the **exit gate opens**; reaching it ends the room ("INSTALL COMPLETE") and returns to the RACE (or, after the last epoch, the boss).
* Tunables: `HACK_FRAMES[epoch]`, zone size, reset-or-decay.

---

## 4a. Room movement: Mario-style on the hoverboard

Decision 2026-10-12: no tail-hang. Geeko moves like a platformer hero, with the hoverboard's **inertia on start and stop** (it keeps gliding after you let go, like the board in the RACE).

Proposed physics (8.8 fixed point like the runner; every value is a tunable constant):

| | Ground | Air |
|---|---|---|
| Run acceleration | +0x18 per frame to a max of 0x200 (2 px/frame, about 21 frames to full speed) | +0x0C (weaker steering) |
| No input | friction -0x10 per frame (about 32 frames to a stop: the glide) | none (momentum kept) |
| Reversing | skid: -0x30 per frame while the board turns | steer at the air rate |
| Jump | A: impulse -0x4C0 (the RACE value); letting go of A while rising cuts the upward speed (variable height) | gravity 0x38 (the RACE value) |
| Optional | "coyote time" (about 4 frames of jump after leaving a ledge) | |

* **Hacking:** while B is held the D-pad is ignored, so friction brakes Geeko to a stop; progress fills while the speed is 0 and Geeko is inside the terminal zone. Zones are generous because Geeko slides.
* **Facing left:** the NES flips sprites in hardware (OAM attribute bit 6), so a left-facing Geeko is a second metasprite array with mirrored `dx` and flip set. No extra CHR (the MSX VDP cannot flip, it needs separate patterns).
* New Geeko poses: skid (optional) and hack (reuse the duck pose). Hitbox: reuse `player_get_box()`.
* Platforms and walls come from the room's collision map (section 7.3). Moving platforms carry Geeko.
* Controls in rooms: D-pad left/right = move, A = jump, B = hold to hack, Start = pause. Down is free (crouch/drop through thin platforms, optional).

---

## 5. Boss: THE MAINFRAME

* Single-screen arena: the Mainframe (big eye, static background art, animated eye tile), catwalks, **6 terminals** (3 tiers x left/right, as in the MSX plan).
* The Mainframe **spits AI agents** (crabs) at intervals: they travel along a catwalk or arc toward Geeko. Geeko avoids them (jump, duck, tier hop).
* Each hacked terminal removes one spawn lane and knocks 1/6 off the boss bar.
* All 6 hacked -> overload sequence (eye cracks, flashes, screen shake) -> WIN screen.
* Agent variety (MSX plan, renamed here): top tier fast and erratic (PINCHBOT), middle tier sine wave (WHIRLY), bottom tier bouncing ground patrol (TWINKLE); each is one of the behaviours above. The MSX build only gets one agent on screen at a time and all agents fly straight; here they should really differ. Max 2-3 agents on screen.
* No weapons (MSX: zero combat attacks): agents can only be avoided. Hacking both terminals of a pair permanently removes that agent type. The concept art's tongue zap is not part of the design.

---

## 6. HUD

Concept art shows portrait, health bar, `LIVES xN`, score, inventory slots. Proposed NES HUD (2 text rows, kept above the old split; epoch screens do not scroll, so no split is needed there):

```
YEAR 1990   HACK 1/2   LIVES 3
SCR 000270   HI 000000   SPD 88
```
Boss: top row replaced by a bar of `#` tiles, one per unhacked terminal.

---

## 7. NES constraints and what they force

1. **CHR budget (biggest issue).** NROM has one fixed 8 KB CHR: 256 background tiles (font + road + mountains already use ~70) and 256 sprite tiles (Geeko takes 15, obstacles 12). Four eras plus a boss with distinct terminals, platforms and foes will not fit. Fix: switch mapper so CHR is **banked per epoch**; the warp flash is a natural place to swap. Which mapper: section 7a.
2. **Sprites per scanline: 8.** Geeko is 3 tiles wide. In an epoch screen: Geeko 3 + two 2-tile foes = 7. The boss with 2-3 crabs needs care; fallback is a narrower Geeko variant in the arena. Draw order already puts the least important sprites last.
3. **Player physics change.** The runner has a fixed X and a jump arc only. Rooms need left/right with inertia, jump, and platform collision against a **per-screen collision map** (1 bit per 8x8 tile = 120 bytes per screen); see section 4a. Facing left uses the hardware sprite flip.
4. **Level data.** Raw nametable = 960 bytes + 64 attributes per screen; 6 screens = ~6 KB raw, fine in 32 KB PRG even uncompressed (RLE later if needed).
5. **RAM 2 KB:** terminals (state + progress, max 6), foes (max ~4), fine.
6. **Audio.** Today: raw APU pokes in `sfx.c`, no music. The SDK ships a real engine, see section 7b.
7. **Testing.** Mesen Lua can poke RAM, so tests can jump straight to a state (e.g. write `mph = 88`, or set the epoch), then assert hack timing, life loss, boss win. `make test` is the harness.

---

## 7a. Mapper alternatives in the current compiler

Checked 2026-10-12 in the llvm-mos SDK of `Containerfile` (v23.3.0): targets `mos-nes-<mapper>-clang` and their linker scripts under `mos-platform/nes-<mapper>/lib`. Limits below are the SDK's own `ASSERT`s, not the hardware's.

| Target (mapper) | CHR | PRG | What it gives us | Notes |
|---|---|---|---|---|
| `nrom` (0), **current** | up to 8 KiB ROM or RAM | fixed | simplest | too small for per-era art |
| `cnrom` | CHR-ROM in **8 KiB banks**, up to 2048 KiB | fixed (no PRG banking) | `set_chr_bank(n)`; `swap_chr_bank(n)` / `split_chr_bank(n)` change the bank at the next NMI / until the next NMI. BG and sprite tables swap together | Lowest-effort fix. Each bank repeats the font + HUD tiles (~50). Bus conflicts handled by the SDK's rompoke table (`_rompoke.ld`) |
| `mmc1` | CHR-ROM up to 128 KiB in **4 KiB banks** (BG and sprites independent), or CHR-RAM up to 8 KiB | banked (`set_prg_bank`, `banked_call`) | independent BG / sprite banks, PRG banking, PRG-RAM up to 32 KiB | More code than CNROM; needs `banked_call` discipline |
| `mmc3` | CHR-ROM in **1 KiB banks** (finest) | up to 512 KiB, banked (`set_prg_8000/a000`, `banked_call*`) | per-1 KiB CHR swaps and a **scanline IRQ** (`set_irq_ptr`, `is_irq_done`), so the HUD split would not need sprite 0; PRG-RAM up to 8 KiB, mirroring control | Most capable and the most complex; widely supported by emulators and flash carts (the `file-fixers` sibling uses it) |
| `unrom` | 8 KiB (in practice CHR-RAM) | banked, up to 4096 KiB | big PRG; with CHR-RAM, tiles are uploaded from PRG at warp time (screen off) | Art lives in PRG, so no CHR-ROM size limit, but needs an upload routine |
| `unrom-512` | ROM or RAM up to 32 KiB | up to 512 KiB, banked | mirroring control, homebrew-board friendly | CHR-RAM like UNROM |
| `gtrom` | CHR-RAM up to 16 KiB | up to 512 KiB (32 KiB banks) | 2 nametable banks, board LEDs | Homebrew board |
| `action53` | up to 32 KiB (ROM or RAM) | banked (inner/outer) | multicart mapper; `mbh-A53-witchnwiz` uses it | Aimed at compilations |
| `mos-nes` (generic) | - | - | base platform with no mapper code | Not a game target |

All of them link the same `neslib`, `nesdoug` and FamiTone2 libraries. A different mapper changes the compiler command (`CC=mos-nes-<mapper>-clang`), the linker section names for CHR (`chr_rom_0`, `chr_rom_1`, ... for banks) and the iNES header; `build_chr.py` / `chr-rom.s` would emit one bank per epoch. Not prototyped: bank section names for `.incbin` and the bus-conflict handling still have to be tried.

**Recommendation:** CNROM first (smallest change, one `set_chr_bank()` at the warp flash). If the game later needs finer banking or no sprite-0 HUD split, step up to MMC3. Nothing in the loop or the modules depends on the mapper.

---

## 7b. Music and sound effects

**What we have today.** `sfx.c` writes the APU registers directly (Pulse 1 jump/duck, Pulse 2 beep, Triangle warp, Noise crash). There is no music.

**What the current stack already includes** (checked in the container, 2026-10-12):
* **FamiTone2** (Shiru): `neslib.h` includes `famitone2.h`; `libfamitone2.a` is in the SDK. API: `music_init` / `music_play(n)` / `music_stop` / `music_pause`, `sounds_init` / `sfx_play(id, channel 0..3)`, `sample_play(n)` for DPCM, and `banked_music_init` / `banked_sounds_init` for banked mappers. `nesdoug` adds `set_music_speed`. The examples never call an update function, so the engine is driven from the NMI (inferred).
* **Converters in the container**: `ft2-text2data` (FamiTracker text export -> music data) and `ft2-nsf2data` (NSF -> sound-effect data), "famitone version 2d". Linux-native, no Wine.
* **DPCM**: `.dmc` samples and a linker script `dpcm.ld` (samples must sit late in ROM, at or above `$C000`).

**Verified workflow** (reproduced here against the sibling `nesdoug-llvm/18_Music`): compose in **FamiTracker** (or Dn-FamiTracker) -> export text `song.txt` -> `ft2-text2data song.txt -ca65` -> `sed 's/@/.L/g'` (LLVM assembler wants `.L` local labels; with it the output is byte-identical to `nesdoug-llvm`'s checked-in `TestMusic3.s`) -> `.s` file included from a small `music.s` that defines `music_data` in `.rodata.music_data` (same for `sounds_data` from the NSF) -> `music_init(music_data); sounds_init(sounds_data); music_play(0); sfx_play(id, 0);`. Link with `-lfamitone2`. Sound effects must stay small (255 bytes each), and DPCM cannot be used inside SFX.

**Options**

| Option | Verdict |
|---|---|
| Keep raw APU pokes | Fine for 5 effects, no music. Cannot coexist with an engine on the same channels |
| **FamiTone2** (bundled) | **Recommended.** Works today in this toolchain, converters included, reference code in `nesdoug-llvm` (18_Music, 19_SFX, 20_DPCM). Migrate `sfx.c` to `sfx_play` so effects and music share the APU properly |
| FamiTone 5.0 | Newer FamiTone variant with its own converters (`text2vol5.exe`, `nsf2data5.exe`, Windows `.exe`, run under Wine by `mbh-A53-witchnwiz`, called by `file-fixers`). Not in the SDK; format differs from the bundled FamiTone2 (not verified compatible) |
| FamiStudio engine | Richer (used by `famidash`: `famistudio_cc65.h`, FamiStudio 4.4.2+, `.dnm` Dn-FamiTracker modules). cc65/ca65 assembly, so it would need porting to LLVM assembler syntax (unknown effort, not tried). FamiStudio can import FamiTracker text, so songs written for FamiTone2 are not lost |
| FamiTracker "BIN" export + own driver | `nes-starter-kit` does music this way (custom `famitracker_driver/*.s`) and FamiTone2 for SFX. cc65 assembly, same porting problem |

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

---

## 8. Roadmap (proposed order)

* **Phase 0: fix the known issues** (section 0). Each fix gets a test:
  1. *Score HUD:* write the digits at column 16 in `hud_update()`. Test: after some frames, read the nametable cells of the HUD row and assert `SCR:` keeps its colon and the digits match `score` (Mesen can read PPU memory; the exact memory type name still has to be checked).
  2. *Drone:* move the flying drone down so its box (y+2, 10 high) overlaps a standing player but not a ducking one: y = 144 gives a box of 146-156 against standing 150-170 and ducking 158-168. Test: turn the `XFAIL` into a `check` and add "ducking player passes under a drone".
  3. *Warp:* implemented in Phase 1 (it needs the new state).
  4. *README:* done 2026-10-15 (current state, controls, build/test commands, link to `plan.md`); revisit when the design lands.
  5. *Sprite overflow:* measure in the emulator (Mesen's state exposes PPU status; the key name has to be checked) while two ground obstacles and the fire trail overlap Geeko; add a test if it is cheap.
  6. *Makefile `CC`:* default to the container build (a `make podman` target) or document `CC=`.
  7. *Font:* add `'` and `,` (and lowercase when the story screens need it) to `FONT_RAW` in `build_chr.py`.
* **Code layout (done 2026-10-12).** `src/` is split into modules (`player`, `obstacles`, `progress`, `render`, `hud`, `screens`, `sfx`; see AGENTS.md). New states are new `state_*` functions in `main.c`; epoch rooms and the boss become their own modules (`epoch.c`, `boss.c`, ...) with the same `*_reset/_update/_draw` shape.
* **Phase 1: sequence + warp.** A sequence table drives the state machine: `STORY`, `RACE`, `WARP`, `ROOM`, `BOSS`, `WIN`. `STATE_WARP`: flash (finally uses `warp_flash_timer`), year counter, then a placeholder room. Text-only story screens (needs the font extended). Test: poke `mph`, assert the state sequence.
* **Phase 2: mapper + audio engine.** Move to CNROM (section 7a): CHR banked, `build_chr.py` outputs banks. Add FamiTone2 (section 7b): `sfx.c` becomes `sfx_play` calls, one test song. Test: `make test` unchanged.
* **Phase 3: room engine.** `struct Room` (start, exit, terminals, platforms, hazards, foes, layout), Mario-style movement with inertia (section 4a), collision map, terminal + hold-B hack meter, exit gate, return to RACE. Test: hold B N frames -> hacked -> gate opens.
* **Phase 4: foes and hazards.** Patrol, stationary hazard, projectile, moving platforms, mines; lives; vision cone for the guard (optional).
* **Phase 5: era content.** 1990, then 2000, 2010, 2020: tiles, terminals, foes, level layout.
* **Phase 6: boss.** Arena, 6 terminals, spitting agents, overload.
* **Phase 7: polish.** Title (the box art), ending, era music, README/AGENTS.md.

---

## 9. Decisions

Decided (your messages of 2026-10-12):
* **RACE + HACK** both stay in the game; the runner is the way into each epoch.
* **No combat** (MSX design); no tongue.
* **Damage:** 1 hit = lose a life (a health bar from the concept art stays optional).
* **Hack = hold B.**
* **No tail-hang.** Rooms use Mario-style jumping on the hoverboard, with inertia on start and stop (section 4a).

Still open:
1. **Mapper:** CNROM (recommended) vs MMC3 / MMC1 (section 7a).
2. **Music engine:** FamiTone2 (recommended, already in the SDK) vs FamiStudio (section 7b).
3. **4th epoch:** 2020 (brief) vs 2054 (MSX world 4). Plan assumes 2020, with the Mainframe as the 2054 AI era.
4. **Rooms per epoch:** 1 for the first playable version, more later (MSX: 8).
5. **Terminals per room and hack times:** values in section 3 are guesses; so are the physics constants in section 4a.
6. **Geeko in rooms:** keep the 24 px sprite (budget: section 7.2) or draw a smaller one.
7. **Story screens:** how many and how long; they need a lowercase/punctuation font (currently only `A-Z 0-9` and a few symbols, and `'` / `,` render blank).
8. ~~**Licence of this repo:**~~ **Decided 2026-10-15: GPL-3.0** (`LICENSE`). See section 10.
9. **Branding:** the box art is MSX-branded; the concept art uses the openSUSE logo and AI-company-named agents (Clawd / ChatGPT / Gemini in the MSX plan). Proposed renames: **PINCHBOT** (orange blocky crab), **WHIRLY** (teal swirl), **TWINKLE** (cyan star), redrawn rather than copied (`concept-art/PROMPTS.md`). Check the openSUSE logo guidelines (the MSX plan says the same).

---

## 10. Licences of the neighbouring projects

Checked 2026-10-12 from each project's `LICENSE` / README in `nesdev/` (siblings of this repo) plus the MSX project. Licence files cover the repo's *code*; art, music and levels can carry their own terms, noted where the README says so.

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
| MSX sibling (`msxdev/hack-to-the-future`) | Code **GPL-3.0**; assets **CC BY-NC-SA 4.0** (Associazione MSX Italia), "derived from Chrono Runner" | Ideas free. Copying code means GPL-3.0 for the result; copying assets means attribution + non-commercial + share-alike, and Chrono Runner's own provenance |

What this means for this repo (`LICENSE` is GPL-3.0 since 2026-10-15; the bullets below are the reasoning that led there):
* Nothing here is *forced* on us as long as we copy no code or assets. We already ship MIT / public-domain library code (nesdoug, neslib, FamiTone2) in the ROM: keep a credits file with their notices.
* If code or data is copied from the MSX project, this repo becomes GPL-3.0 (code) and CC BY-NC-SA 4.0 (assets). That also rules out commercial use.
* If we stay ideas-only, any licence works. MIT would match most of the neighbours; GPL-3.0 would match the MSX sibling and keep the two projects mergeable. **Chosen: GPL-3.0.** MSX *code* can now be shared both ways; MSX *assets* (CC BY-NC-SA 4.0, non-commercial) still need their own terms and are not covered by the repo licence.
* The MIT/CC0 projects (`file-fixers`, `witchnwiz`, `nes-starter-kit`, `nesdoug-llvm`) are safe sources if we want a reference implementation to copy from; `from-below` and `famidash` content are not.
