# Hack to the Future (NES) - Game Concept & Implementation Plan

Geeko (the openSUSE chameleon) races on an anti-gravity hoverboard until he hits **88 MPH**, travels through time, and **hacks the future**: in each IT era he must hack every terminal and install openSUSE, avoiding era-themed foes. Final boss: **The Mainframe**.

**Two halves, one game** (decision 2026-10-12):
* **RACE**: the NES version's own endless runner (already built): jump and duck on a road while the speed climbs from 20 to 88 MPH.
* **HACK**: rooms with terminal hacking, the six-terminal Mainframe boss and story screens.

Numbers marked *proposed* are starting values to tune, not decisions.

---

## 0. Status (checked 2026-10-12, Phase 0 fixes 2026-10-19)

Verified by reading the code, `make test` (headless Mesen2, see AGENTS.md) and screenshots.

| Piece | State |
|---|---|
| Endless runner (road, jump/duck, 3 obstacle types, score, 20 -> 88 MPH) | Works, smoke-tested |
| Geeko sprite (16x16 riding and ducking, striped chameleon on a white hoverboard; redrawn 2026-10-16, 24x24 before) | Done |
| HUD with sprite-0 split | Works (split bug fixed, regression-tested) |
| Time warp at 88 MPH | **Only a sound** (known issue 3) |
| Epoch screens, terminals, hacking, era foes, boss | Not started (step list: section 8) |
| Mapper | **MMC3 decided 2026-10-19**; spike works (section 7a); the port is step 1.1 |

### Known issues (not fixed)
1. ~~**Score HUD shows score x 10.**~~ **Fixed 2026-10-19** (column 16, tested). Was: `hud_update()` (`hud.c`) writes the 5 digits at column 15, but the static text `SCR:00000` has its digits at 16-20. The colon is overwritten and a stale `0` stays at column 20 (code + screenshot: `SCR000270`). Fix: column 16.
2. ~~**A standing player is not hit by a drone.**~~ **Fixed 2026-10-19** (drone y 147; standing is hit, ducking passes, tested). Was: Drone hitbox y138-147, standing hitbox starts y150 (confirmed in the emulator, `XFAIL` in `tests/smoke.lua`). "Duck under drones" currently does nothing.
3. **The 88 MPH warp is only a sound.** `warp_flash_timer` is set to 30 in `progress.c` and never read or decremented: no flash, no transition.
4. ~~**`README.md` is out of date.**~~ **Fixed 2026-10-15:** rewritten (current state, controls, build/test, licence).
5. ~~**8 sprites per scanline not measured.**~~ **Measured 2026-10-19:** worst case playing at 88 MPH is exactly 8 (tested); crashed at 88 MPH with 2 obstacles it is 9 and the trail's second flame is dropped (cosmetic). Was: Geeko is 2 tiles wide now (was 3); 2 ground obstacles + Geeko + fire trail should stay at 8 or less, but nobody has measured it. Draw order protects obstacles (trail is last), but nobody has looked at it in the emulator.
6. ~~`Makefile` hardcodes `CC`~~ **Fixed 2026-10-19:** `make podman` builds in the container; `CC=` still overrides.
7. ~~The font has no `'` or `,`~~ **Fixed 2026-10-19** (glyphs added; a test fails on any title character without a glyph).

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
* **ROOM is the hack level**: one single screen (no scrolling) with platforms, terminals, foes, hazards and an exit. Geeko rides the hoverboard Mario-style: run with inertia, jump (section 4a). No weapons.
* The game is driven by a **sequence table**: a list of `STORY`, `RACE`, `ROOM(n)`, `BOSS` entries; main.c steps through it. Adding content = adding rows.
* **No death** (decisions 2026-10-19). There are no lives and no health. A hit costs **fixed time** from the clock: a crash in the RACE (which also slows Geeko, so reaching 88 MPH takes longer), contact with a foe in a ROOM, a boss agent. Hacking a terminal **adds time**. There is **one clock for the whole game, 10 minutes** (decision 2026-10-19). The only game over is **the clock reaching 0** (a penalty can take it there); what it restarts is *proposed*: the whole game from the title, with a full clock (D6). The amounts are *proposed* constants (`HIT_TIME_PENALTY`, `HACK_TIME_BONUS`, decision D9): section 8 steps 2.3, 2.4, 4.6, 5.2.
* Rooms per epoch: 1 for the first playable version; the room format allows several.

---

## 2. Concept art (`concept-art/`)

Eleven images, named by content (renamed from `Gemini_Generated_Image_*` on 2026-10-15). The image files are not in the repo (`*.jpeg` is gitignored); they are linked from an external site. Four were inspected on 2026-10-12 (they existed as low-res `17917…jpg` plus a 2752x1536 copy; the low-res copies have since been removed) and are catalogued below. The other seven (renders from `concept-art/PROMPTS.md` and NES-style mock-ups, `nes-*` and `illustration-race-*` / `illustration-1990-*`) are identified by filename only.

| File | Shows | Design takeaways |
|---|---|---|
| `box-cartridge-manual-photo.jpeg` (a photo mock-up of box + cartridge + manual; the flat cover art was `1791699937579.jpg`, now removed) | Box art "ゲーコング" by **GEEKO SOFT**. Geeko with goggles, thumbs up, spiral tail, sitting on a CRT between tower PCs; green binary rain over server racks; orange diagonal banner. retro home-computer styled, not NES. | Tone and palette (black, openSUSE green, orange, CRT cyan glow). A NES box/cart label would need a re-brand. Geeko wears goggles; ours does not (cheap to add) |
| `illustration-server-room-tongue-hack-guard-drone.jpeg` | **1990 stage.** Two-tier steel catwalk with railings. Geeko crouches on the upper tier and hacks a beige tower + CRT showing a green `>_ openSUSE` prompt, with cyan particles. A server at the right shows a **blue screen**. Below: a hovering **security drone** and an **armoured guard**, each with a red **vision cone**. HUD: portrait + health bar, `LIVES x3`, score, inventory slots (tower/terminal/etc. with counts), tool slots (wrench, hourglass) | Unhacked = blue screen, hacked = green terminal (cheap, readable state change). Vision cones imply stealth: foes are dangerous when you are in the cone. Two tiers fit one NES screen |
| `illustration-tail-hang-laser-grid-laptop.jpeg` | **2010 stage.** Geeko **hangs by its tail** from an overhead pipe and hacks a **laptop** on a hovering platform (openSUSE logo on screen). Hazards: red **laser tripwires** across the floor, a sparking ceiling machine. HUD: health + energy bars, lives, score, wrench + laptop slots, openSUSE badge | The ceiling hang is not ported (jumping instead). Hazards that are not creatures (lasers, sparks) are cheap to implement and era-flavoured |
| `illustration-mainframe-boss-tongue-vs-crab.jpeg` | **Boss: THE MAINFRAME.** A monolith with a huge red slit-pupil eye, a teal swirl emblem, red circuit lines, smoke, flanked by server racks. Geeko leaps along a catwalk; its tongue is a magenta lightning bolt hitting an **orange pixel crab** (AI agent, shown glitching into green code); a second crab hovers; a cyan star sparkle. HUD: boss bar "THE MAINFRAME", player health + energy, lives, score | Boss = one static, big, readable sprite group + projectiles. The boss bar can be the count of unhacked terminals. The crab matches the "Clawd" agent (likely; the art does not say) |

Conflicts between the art and this plan:
* The art hacks with the **tongue** (and uses it as a weapon on the boss). The plan: **no weapons**, hack by standing in a zone and holding B. The tongue is not part of the sprite any more.
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

* Moods: 1990 UNIX labs / BBS (warm red brick, CRT glow); 2000 dot-com server farm (steel racks, blue LEDs, firewalls); 2010 cloud (white cleanroom panels, cyan fibre); 2020 / the boss: AI Citadel (dark chassis, neon green traces).
* Era entry: warp flash, then a year counter rolls to the era (1985 -> 1990, ...), then the screen loads.
* Foes are era-flavoured re-skins of 4 behaviours, so code stays small: **patrol** (walks a platform), **hover** (sine path), **stationary hazard** (laser, spark, energy field), **projectile**. New foe = new tiles, not new code.

---

## 4. Hacking mechanic

* Stand in the terminal's front zone and **hold B**. A progress meter fills while held. NES proposed 60-120 by epoch (section 3). **Decided: hold B** (your first brief said just stand; holding avoids hacking by accident when sliding past a terminal).
* Progress is drawn on the terminal itself: blue screen -> tile frames filling -> green `>_ openSUSE`. Four background tile frames per terminal type. A tick sound plays every 8 frames while channelling.
* Releasing B or leaving the zone resets progress (*proposed*).
* When every terminal in the room is green, the **exit gate opens**; reaching it ends the room ("INSTALL COMPLETE") and returns to the RACE (or, after the last epoch, the boss).
* **Each terminal finished adds `HACK_TIME_BONUS` to the clock** (decision 2026-10-19; amount *proposed* 10 s, D9); the HUD shows `+10` for a moment.
* Tunables: `HACK_FRAMES[epoch]`, zone size, reset-or-decay, `HACK_TIME_BONUS`.

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
* **Facing left:** the NES flips sprites in hardware (OAM attribute bit 6), so a left-facing Geeko is a second metasprite array with mirrored `dx` and flip set. No extra CHR.
* New Geeko poses: skid (optional) and hack (reuse the duck pose). Hitbox: reuse `player_get_box()`.
* Platforms and walls come from the room's collision map (section 7.3). Moving platforms carry Geeko.
* Controls in rooms: D-pad left/right = move, A = jump, B = hold to hack, Start = pause. Down is free (crouch/drop through thin platforms, optional).

---

## 5. Boss: THE MAINFRAME

* Single-screen arena: the Mainframe (big eye, static background art, animated eye tile), catwalks, **6 terminals**.
* The Mainframe **spits AI agents** (crabs) at intervals: they travel along a catwalk or arc toward Geeko. Geeko avoids them (jump, duck, tier hop).
* Each hacked terminal removes one spawn lane and knocks 1/6 off the boss bar.
* All 6 hacked -> overload sequence (eye cracks, flashes, screen shake) -> WIN screen.
* Agent variety: top tier fast and erratic (PINCHBOT), middle tier sine wave (WHIRLY), bottom tier bouncing ground patrol (TWINKLE); each is one of the behaviours above. Max 2-3 agents on screen.
* No weapons: agents can only be avoided. Hacking both terminals of a pair permanently removes that agent type. The concept art's tongue zap is not part of the design.

---

## 6. HUD

Concept art shows portrait, health bar, `LIVES xN`, score, inventory slots (no health or lives here: decision 2026-10-19). Proposed NES HUD (2 text rows, kept above the old split; epoch screens do not scroll, so no split is needed there):

```
YEAR 1990   HACK 1/2   TIME 10:00
SCR 000270   HI 000000   SPD 88
```
Boss: top row replaced by a bar of `#` tiles, one per unhacked terminal.

---

## 7. NES constraints and what they force

1. **CHR budget (biggest issue).** NROM has one fixed 8 KB CHR: 256 background tiles (font + road + mountains already use ~70) and 256 sprite tiles (Geeko takes 15, obstacles 12). Four eras plus a boss with distinct terminals, platforms and foes will not fit. Fix: switch mapper so CHR is **banked per epoch**; the warp flash is a natural place to swap. Which mapper: section 7a.
2. **Sprites per scanline: 8.** Geeko is 2 tiles wide (16x16). In an epoch screen: Geeko 2 + two 2-tile foes = 6. The boss with 2-3 crabs needs care; fallback is a narrower Geeko variant in the arena. Draw order already puts the least important sprites last.
3. **Player physics change.** The runner has a fixed X and a jump arc only. Rooms need left/right with inertia, jump, and platform collision against a **per-screen collision map** (1 bit per 8x8 tile = 120 bytes per screen); see section 4a. Facing left uses the hardware sprite flip.
4. **Level data.** Raw nametable = 960 bytes + 64 attributes per screen; 6 screens = ~6 KB raw, fine in 32 KB PRG even uncompressed (RLE later if needed).
5. **RAM 2 KB:** terminals (state + progress, max 6), foes (max ~4), fine.
6. **Audio.** Today: raw APU pokes in `sfx.c`, no music. The SDK ships a real engine, see section 7b.
7. **Testing.** Mesen Lua can poke RAM, so tests can jump straight to a state (e.g. write `mph = 88`, or set the epoch), then assert hack timing, time penalty, boss win. `make test` is the harness.

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

**Decided 2026-10-19: MMC3.** Only MMC3 has a scanline IRQ among the eight SDK targets (the other `mapper.h` files have none), so it is the only one that removes the sprite-0 trick; it also has 1 KB CHR banks, so the font and Geeko tiles are not repeated per era.

**Spike (2026-10-19, `/tmp/mmc3-spike`, a throwaway copy, not in the repo)**: the current game built with `mos-nes-mmc3-clang` and ~30 changed lines (init in `main.c`, `hud_scroll()` building the IRQ array, sprite 0 disabled). Measured in Mesen:
* `tests/smoke.lua` passes unchanged, including the scroll split on scanline 23. IRQ array `{22, 0xF0, ctrl, 0xF5, x, 0xFF}`: the IRQ handler's two `$2005` writes land on scanline 23, dots 35-57, before dot 257, so the new coarse scroll applies from line 24. With count `23` they land on line 24 (the first playfield line, black sky: invisible here, but a glitch for other art).
* Game logic starts at scanline 3 instead of 29 (`scroll_x` write observed over 59 frames in each ROM; MMC3: scanline 2-4 in most frames, up to 12 in a few): about 26 scanlines (roughly 3,000 cycles, 10% of a frame) are no longer spent spinning on the sprite-0 flag. On NROM the spin ran from scanline 258 to 23.
* Default ROM image is **768 KB** (header: 512 KB PRG, 256 KB CHR). `MAPPER_PRG_ROM_KB(64); MAPPER_CHR_ROM_KB(8);` (from `ines.h`) gives 73,744 bytes, header PRG 4 x 16 KB, CHR 1 x 8 KB, mapper 4, and all tests pass. The link script asserts PRG greater than 32 KB.
* `set_chr_bank` "may have no effect if interrupted"; use `set_chr_bank_retry` outside vblank-safe code. The IRQ array has no CHR commands, so main-thread CHR changes are safe (comment in `nesdoug-llvm/33_MMC3/MMC3/mmc3_code.asm`).
* Not checked: real hardware (MMC3A / MMC3B IRQ differences matter for reload value 0, not used here), other emulators, how the SDK restarts the IRQ array each frame (observed to work, source not read).

Nothing else in the loop or the modules depends on the mapper.

---

## 7b. Music and sound effects

**What we have today (draft since 2026-10-16).** FamiTone2 plays one track and the five effects: `sfx.c` calls `music_play` / `sfx_play`, the data is in `assets/audio/` (generated by `tools/export_audio.sh`, see below). The old raw APU pokes are gone (the engine rewrites every channel each frame; the raw crash noise had also rung forever, fixed by the move). Draft limits: the track is `nes_23-chill`, a placeholder, with its volume-column echo lost (FamiTone2 has no volume column; `/tmp/music-listen/chill_famitone2_approx.wav` is FamiStudio's approximation of what plays); the music plays only in `STATE_PLAYING` and stops on the crash; the title screen is silent; the effects were redrawn in FamiStudio text (`assets/audio/sfx.txt`): jump = rising chirp, duck = falling chirp, beep = high ping, warp = rising whoosh (new, the old one was a short tone), crash = noise burst that fades in under a second. Nobody has listened to the effects in the game yet. The 88 MPH warp still has no music change.

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

**Mapper decided 2026-10-19: MMC3** (spike in section 7a). Phase 0 (bug fixes) is done (2026-10-19, `DIARY.md`); the 88 MPH warp moved to M2. The milestones below are ordered so the game is **playable after every milestone**; the cut lines at the end say where to stop if time runs out. Hack Week 26 is 2026-11-30 to 12-04.

Rules for every step: it ends with `make test` green; logic gets a check (`tests/smoke.lua` or a script next to it); a change that must not alter the picture is checked with `tests/trace.lua` (diff identical); one `DIARY.md` line; commit only when asked. Sizes are relative (S, M, L), not time: nobody knows how much time there is.

**Budgets** (ELF sections of the MMC3 spike, 2026-10-19; keep re-measuring with `llvm-readelf -S -W hacktothefuture.nes.elf`):

| Resource | Now | Limit / target |
|---|---|---|
| RAM | about 0.7 KB (`.aligned` 416 B incl. the 256 B OAM buffer, `.noinit` 185 B, zero page about 120 B); the C software stack is not counted | 2 KB. Room layouts, foe lists and text must be `const` in PRG, never copied to RAM |
| PRG | about 8.3 KB (`.text` 6.9 KB + `.rodata` 1.4 KB, includes the audio data) | 64 KB image (`MAPPER_PRG_ROM_KB(64)`). Where the default code placement ends is **not checked**: when `.text` + `.rodata` pass about 20 KB, check it and move room data and story text to banked sections (`#pragma clang section`, as in `nesdoug-llvm/33_MMC3`) |
| CHR | 8 KB | 32 KB target: 4 KB common + 4 KB per era (M3) |
| CPU | game logic starts at scanline 3 on MMC3 (29 on NROM, measured) | the IRQ split costs a few hundred cycles; watch the "no dropped frames" check |

### M1. MMC3 port (platform)

| # | Step | Done when | Size |
|---|---|---|---|
| 1.1 | Port as in the spike: `main.c` init (`disable_irq`, `CLI`, PRG mode 0, `CHR_A12_INVERT`, 6 CHR banks, `MIRROR_VERTICAL`, IRQ pointer); `hud_scroll()` builds `{22, 0xF0, ctrl, 0xF5, x, 0xFF}` (derive `ctrl` from the neslib PPU control value instead of the spike's hardcoded `0x88`); `MAPPER_PRG_ROM_KB(64)`, `MAPPER_CHR_ROM_KB(8)` | `make test` unchanged and green; ROM is 73,744 bytes (spike) | S |
| 1.2 | Delete sprite 0: `hud_sprite_zero`, `hud_draw_split_target` (and its call in `screens.c`), the `oam_set(4)` reserve in `render.c`, tiles `0x23` / `0x89` in `build_chr.py` | `make test` green; new `tests/trace.lua` baseline recorded (hashes change: the target pixel is gone) | S |
| 1.3 | `CC` default and `make podman` use `mos-nes-mmc3-clang`; `AGENTS.md` (runtime model, build), `plan.md` 7a, `hud.h` comment describe the IRQ split | docs match code | S |
| 1.4 | Run the ROM in a second emulator (Nestopia, `AGENTS.md` Run) and look at the HUD; real hardware if a cart is available | no garbled HUD row (only the user can judge this; Mesen is the only emulator checked) | S |

### M2. Game loop skeleton (old Phase 1)

| # | Step | Done when | Size |
|---|---|---|---|
| 2.0 | Test harness: move `check`, `wait`, `press`, `poke`, symbol lookup out of `smoke.lua` into `tests/lib.lua`, plus `tuning("NAME")` which reads a numeric `#define` from `src/tuning.h` (tests follow the tuned values); `make test` runs every `tests/*.lua` except `trace.lua` | same output as before | S |
| 2.1 | Sequence table: `struct Step {kind, arg}` rows (`STORY`, `RACE`, `WARP`, `ROOM`, `BOSS`, `WIN`) and a `seq_index`; `main.c` runs the current row. Behaviour unchanged (title, race, game over) | `trace.lua` diff identical; a test pokes `seq_index` to jump to any row | M |
| 2.2 | Font: add `>` `_` `?` (terminal prompt `>_`, questions) and whatever the story text needs; decide lowercase (D3). Lowercase would sit at `0x61-0x7A`, in a common CHR bank | the glyph test scans every screen state, not only the title | S |
| 2.3 | **Race without death.** A crash no longer ends the run. Decouple `mph` from `score` (today `mph = 20 + score/40`: +1 MPH per 160 frames, so 181 s to reach 88 MPH without a crash). `mph` rises on its own rate; a crash subtracts `CRASH_MPH_LOSS`, starts `INVULN_FRAMES` (Geeko blinks), plays the short crash effect; the music keeps playing; `speed_px` follows `mph`, so obstacles slow down too. **A clean race takes 60 s (decided 2026-10-19)**: `RACE_FRAMES_PER_MPH` 53 gives 68 MPH x 53 = 3,604 frames. Constants live in `src/tuning.h` as plain `#define`s (decided 2026-10-19; the file exists, nothing includes it yet): `CRASH_MPH_LOSS` 10 (floor 20; the slowdown alone costs about 9 s) and `INVULN_FRAMES` 60 are *proposed*. At today's rate the same loss would cost 27 s. The fixed time penalty of a crash is added in 2.4 (needs the clock): then a crash costs the penalty plus the slowdown, about 14 s at the proposed values | tests: a crash keeps the state at PLAYING and drops `mph`; an obstacle during the invulnerability does not hit again; a run with the spawn timer held (no obstacles) reaches 88 MPH in the proposed time (raise `MAX_FRAMES`, 4000 today, 3,600 frames is 60 s); a run that keeps hitting obstacles is slower. **These smoke checks are rewritten:** crash into a ground obstacle, crash into a drone, "idle player crashes", "all sound stops after the crash", the 88-MPH sprite measurement (needs no game over) | M |
| 2.4 | **Clock.** One clock for the whole game, `GAME_TIME_S` = 600 (10 minutes) from `tuning.h`. `time_left` is a u16 count of frames: 600 s = 36,000 frames fits (the most it can hold is 65,535 frames = 1,092 s; `tuning.h` has an `#error` if `GAME_TIME_S` or `TIME_MAX_S` goes above that). Shown in the HUD as `TIME mm:ss`, redrawn only when the second changes (no division every frame); counts down in RACE and ROOM only (frozen on title, story, warp). API `clock_sub(s)` / `clock_add(s)` take seconds: saturates at 0 (game over) and at `TIME_MAX_S`; the HUD shows the change (`-5`, `+10`) for about a second. The race crash calls `clock_sub(HIT_TIME_PENALTY_S)`, once per crash (not during the invulnerability); `clock_add` is used by the terminals (4.6). At 0: game over (`TIME UP` banner replaces `CRASH!`, music off), Start restarts the whole game with a full clock (D6, *proposed*). `progress_game_over()` and the hi-score move here | tests: poke `time_left` to 120 (2 s): game over within about 2 s; a crash lowers `time_left` by exactly `HIT_TIME_PENALTY_S` x 60 (read from `tuning.h`), a second obstacle during the invulnerability costs nothing; a penalty larger than `time_left` ends the game; the HUD shows `10:00` at the start; the clock is frozen during the warp; restart refills it | M |
| 2.5 | `STATE_WARP`: use `warp_flash_timer` (now unused): palette flash, year counter 1985 to 1990 as text, obstacles frozen, `sfx_warp`; trigger at 88 MPH | test pokes `mph` to 88 (after 2.3 `mph` is its own variable): RACE, WARP, next row, within a frame budget | M |
| 2.6 | Placeholder `ROOM`: a screen that says `ROOM 1990` and returns on Start; epoch number in RAM; HUD shows `YEAR` | loop RACE, WARP, ROOM, RACE (next epoch) works in a test | S |
| 2.7 | Story screens: text-only renderer `story_show(id)` and a text table (intro, one per epoch, pre-boss, ending; placeholder text) | each screen appears in sequence and Start advances; glyph test | M |

### M3. Era graphics foundation (MMC3 banking)

With `CHR_A12_INVERT`: R0 = `$1000-17FF` (2 KB), R1 = `$1800-1FFF` (2 KB), R2 `$0000-03FF`, R3 `$0400-07FF`, R4 `$0800-0BFF`, R5 `$0C00-0FFF` (1 KB each; from `mmc3_code.asm`). Current tiles map onto this: font `0x20-0x5F` is in R2 and R3 (fixed, common); road, sky, mountains `0x80+` are in R4; Geeko, obstacles and FX `0x00-0x23` are in R0 (fixed, common). **Per era: R4 + R5 (BG tiles `0x80-0xFF`) and R1 (sprite tiles `0x80-0xFF`)**, three register writes. About 64 BG tiles stay free in the common banks (`0x00-0x1F`, `0x60-0x7F`).

| # | Step | Done when | Size |
|---|---|---|---|
| 3.1 | `build_chr.py` emits 1 KB banks with a named layout (common BG, common sprites, era N BG, era N sprites); `chr-rom.s` includes the bigger file; `MAPPER_CHR_ROM_KB` raised | `graphics.chr` size asserted; ROM boots | M |
| 3.2 | `chr_set_era(n)`: the three writes, done during the warp flash. Use `set_chr_bank_retry` (a plain `set_chr_bank` "may have no effect" if interrupted; the HUD IRQ shares the register). Safe because the HUD IRQ array has no CHR commands | test: tile `0x80` read through `nesPpuMemory` differs between era 0 and era 1 after the warp | S |
| 3.3 | Two throwaway era tilesets (recolour of the current one) to prove the swap in a screenshot | screenshot differs; palette per era in `palettes.h` | S |

### M4. Room engine (old Phase 3): one era, one room, no foes

Design of the rooms themselves (maps, foes, numbers): section 8a. New modules: `room.c` (data, loading, tile lookup), `hero.c` (Geeko in rooms; `player.c` stays the race), `terminal.c`. A room is one screen (32x30 tiles), HUD in rows 1-2 like the race, no scroll (IRQ array `{0xFF}`).

| # | Step | Done when | Size |
|---|---|---|---|
| 4.1 | `struct Room` in PRG (format and tile classes: 8a.3): palettes, 960-byte nametable (raw; RLE only if PRG gets tight), attributes, start, gate, terminal list, foe list. **Solid tiles = a tile-index class**: `room_solid(t) = (t & 0xE0) == 0xC0`; `room_solid_at(tx, ty)` reads the nametable in PRG. This replaces the separate 120-byte collision map of 4a: nothing to keep in sync, no RAM | loading a room: the nametable read back through `nesPpuMemory` equals the data | M |
| 4.1a | `tools/build_rooms.py`: rectangles and object lists (the format of section 8a.5) to nametable, attributes and const arrays; ASCII preview; reachability check against the 8a.2 rules (`tools/check_rooms.py` already holds the room rectangles and the check; the generator extends it) | the five rooms of 8a/8b build; the check passes for all | M |
| 4.2 | `room_load(id)`: `ppu_off`, palette, nametable, HUD text, IRQ array `{0xFF}`, scroll 0, CHR era | the ROOM state shows the real room (screenshot) | S |
| 4.3 | `hero.c` horizontal movement with inertia (4a constants: accel `0x18`, max `0x200`, friction `0x10`, skid `0x30`), no collision yet | test measures from RAM: frames to full speed (about 21) and stopping distance | M |
| 4.4 | Gravity, jump with variable height, tile collision per axis (floor, walls, ceiling), falling off ledges | tests: lands on a platform; blocked by a wall; walks off a ledge and falls | L |
| 4.5 | Facing: left-facing metasprites (mirrored `dx`, flip attribute `0x40`), hack pose = the duck pose | screenshot of both directions | S |
| 4.6 | Terminals: zone, **hold B** hack meter (progress drawn as BG tiles through the VRAM buffer), D-pad ignored while holding, release resets, tick sound (new effect in `sfx.txt`, `tools/export_audio.sh`); on completion `clock_add(HACK_TIME_BONUS)` and a sound | test: hold B `HACK_FRAMES` in the zone gives hacked and raises `time_left` by the bonus, once per terminal; release resets; outside the zone does nothing | M |
| 4.7 | Exit gate: opens when every terminal is hacked (tile swap), reaching it shows `INSTALL COMPLETE` and advances the sequence | test: hack all, walk to the gate, next row | S |
| 4.8 | **Room 1990 v1** (8a.5 room 1): art for catwalk, tower, CRT, terminal states, gate (era 1 banks), the drone | a recorded-input playthrough test completes the room (foe movement is table-driven, so it is reproducible) | L |
| 4.9 | Pause on Start | test: state frozen while paused | S |

### M5. Foes and hazards (old Phase 4)

| # | Step | Done when | Size |
|---|---|---|---|
| 5.1 | `foe.c`: up to 4 slots, `struct Foe` with `x, y, width, height` first (for `check_collision`), 4 behaviours (patrol, hover on a sine table, stationary hazard, projectile) driven by the room's foe list | each behaviour has a test that reads its position over N frames | L |
| 5.2 | Hero vs foe: contact costs `clock_sub(HIT_TIME_PENALTY)` (**decided: fixed time**; amount *proposed* 5 s), knocks Geeko back and gives 1 s of invulnerability (*proposed*; the invulnerability is required, contact is tested every frame and would drain the clock 60 times a second). No death, no respawn; only the clock ends the game | tests: a touch lowers `time_left` by the penalty and pushes Geeko away; a second touch during the invulnerability costs nothing | M |
| 5.3 | Sprite budget in rooms | the OAM per-scanline check from `smoke.lua` also runs in the worst room: at most 8 | S |
| 5.4 | Moving platforms (carry Geeko) and mines, only when an era needs them (2020 and 2010) | test: Geeko on a moving platform moves with it | M |
| 5.5 | Guard vision cone | **cut by default** | - |

### M6. Era content (old Phase 5)

Repeat per era; 1990 first because 4.8 already built most of it.

| # | Step | Done when | Size |
|---|---|---|---|
| 6.1 | Era recipe: BG and sprite tiles in `build_chr.py`, palettes, room layout, foe list, story text, warp year; RACE background recoloured per era (optional) | recorded-input playthrough completes the room; glyph and sprite checks pass | M per era |
| 6.2 | 1990: add the drone, the guard (patrol), floppy frisbees (projectile) | as above | M |
| 6.3 | 2000 (Y2K bug, popup, paperclip; falling CRTs) | as above | M |
| 6.4 | 2010 (laptops on platforms; lasers, sparking machine) | as above | M |
| 6.5 | 2020 or 2054 (D2; phones, clouds, bots; moving platforms) | as above | M |

### M7. Boss: The Mainframe (old Phase 6)

Design (arena map, agents, lanes, overload): section 8b.

| # | Step | Done when | Size |
|---|---|---|---|
| 7.1 | Arena (8b.1): 3 tiers x 2 terminals on the room engine, Mainframe head and eye as background art, 120-frame hacks, each terminal gives the time bonus | all 6 terminals hackable and reachable in a recorded-input test | L |
| 7.2 | Agents and lanes (8b.2): PINCHBOT, WHIRLY, TWINKLE; lane = (tier, side); fixed 16-entry spawn table, at most 3 on screen; a terminal closes its lane, both of a tier close the type; an agent hit costs the hit penalty | test: lanes close one per terminal (lane flags in RAM); a hit costs the penalty once | L |
| 7.3 | Boss bar in the HUD (8b.3): one `#` per unhacked terminal | test reads the HUD row | S |
| 7.4 | Overload (8b.3): 180 frames of flashes, eye cracks and a room-only shake through the IRQ array, then WIN with the time left | test: hack 6, WIN row reached | M |

### M8. Polish and release (old Phase 7)

| # | Step | Done when | Size |
|---|---|---|---|
| 8.1 | Credits screen with the licence notices (nesdoug MIT, neslib public domain, FamiTone2, the CC0 track) | screen exists, glyph test | S |
| 8.2 | Sound: era music (more songs, same export path), effects for hack tick, hacked, gate, hit | `smoke.lua` audio checks extended | M |
| 8.3 | Title screen art, ending screen | screenshots | M |
| 8.4 | `README.md` and `AGENTS.md` match the finished game; real-hardware or flash-cart run if available | docs reviewed | S |
| 8.5 | Optional: hi-score saved in battery PRG-RAM (`MAPPER_USE_BATTERY`; not tried) | survives an emulator restart | S |

### Open decisions that gate steps

| # | Question | Default if not answered | Needed by |
|---|---|---|---|
| ~~D1~~ | ~~Lives~~ **Decided 2026-10-19:** no death, no lives; a race crash slows Geeko and costs fixed time; hacking adds time; only the clock ends the game | - | - |
| ~~D6~~ | ~~Clock scope~~ **Decided 2026-10-19: one clock for the whole game, 10 minutes** (first 26 for Hack Week 26, then 10 so it fits a 16-bit frame counter). Still *proposed*: a game over restarts the whole game from the title with a full clock | game over restarts the whole game | 2.4 |
| ~~D7~~ | ~~Hit cost~~ **Decided 2026-10-19:** a crash or contact costs fixed time (amount D9). Knockback and invulnerability stay *proposed* | - | 5.2 |
| D8 | What is the score now (it used to drive `mph`)? | bonus for time left at the end of each epoch; shown in the HUD as `SCR`; hi-score kept | 2.3 |
| D2 | 4th epoch: 2020 (brief) or 2054 | 2020 | 6.5 |
| D3 | Story text: uppercase only, or add lowercase | uppercase only | 2.2 |
| D4 | Rooms per epoch (10 minutes fits one room per epoch; more rooms means a longer clock, and the 16-bit counter stops at 18:12) | 1 | 6.1 |
| D5 | Boss agent names (PINCHBOT, WHIRLY, TWINKLE) and openSUSE logo use | proposed names | 7.2 |
| D9 | Time economy. Decided: 600 s total, a race takes 60 s, a hit costs fixed time, a hacked terminal adds time. Open: the amounts (`tuning.h`: penalty 5 s, bonus 10 s, cap 600 s). A flawless run needs about 4:30 of the 10:00 (8a.6); a first-time player at twice my guessed room and boss times is near the limit | keep the proposed amounts | 2.4 |

### Cut lines

* **A, after M2:** the runner with slowdown crashes, the clock, warp, epoch screens and story text; no rooms. Playable and shippable.
* **B, after M4 + 6.2:** vertical slice: RACE, WARP, 1990 room with a foe, INSTALL COMPLETE, back to the RACE. This is the first version that shows both halves of the game.
* **C, after M5 + 6.3-6.4:** three eras.
* **D:** everything, with the boss.

Order of risk: 4.4 (collision and jump feel) and 4.8 (first real level) are the largest unknowns; 3.1/3.2 are the first MMC3-specific code after the port. Do 4.4 before drawing any era art.

---

## 8a. Room design (detailed)

Design of the single-screen hacking rooms (steps M4, M5, M6). Everything marked *proposed* is a starting value. The maps are drafts: they are checked for reachability by `tools/check_rooms.py` (`python3 tools/check_rooms.py -v` prints them; it models the jump envelope below with the constants read from `src/player.c`; it is **not** the emulator), so a real recorded-input playthrough (steps 4.8, 6.x) is still the proof.

### 8a.1 Screen anatomy

* 32 x 30 tiles, one nametable, no scroll (IRQ array `{0xFF}`). Rows 0-2 are the HUD (text in rows 1-2, as in the race). Play area rows 3-27. **Floor top = row 26**; rows 26-29 are solid fill (rows 28-29 can be cropped by a TV, keep nothing important there). Geeko standing on the floor: sprite top y = 26 x 8 - 16 = 192.
* Platform levels every 4 tiles (32 px): floor 26, then 22, 18, 14, 10. Platforms are 1 tile thick. The left and right screen edges are walls (x clamped in code, no tiles).
* **No pits** (nobody dies): the floor is continuous, falling just lands on a lower surface.

### 8a.2 Movement envelope (the numbers the layouts obey)

Simulated with the 4a constants in the same fixed-point order as `player.c` (`vel += gravity; y += vel`, impulse -0x4C0, gravity 0x38); **not yet measured in the emulator** (step 4.4 must confirm them):

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
* **Room data** in PRG: nametable 960 B + attributes 64 B + palettes + terminals, gate, start, foe list: about 1.1 KB per room, 5 rooms about 5.5 KB. Hack frames come from `tuning.h`.
* **Authoring**: rectangles (row range, column range) plus object lists, expanded by `tools/build_rooms.py` into the nametable and arrays, with an ASCII preview and the reachability check (step 4.1a). The rectangles of the maps below are the input format.

### 8a.4 Foes: four behaviours, parameters per room

All 16 x 16, at most 4 per room and at most 3 on the same rows (Geeko 2 sprites + 3 x 2 = 8 per scanline).

| Behaviour | Parameters | Used by |
|---|---|---|
| PATROL | platform range (columns), speed 0.5 px/frame (slow) or 0.75 (fast); turns at the ends | drone, guard, Y2K bug, padlock |
| HOVER | centre, amplitude, period (64-entry sine table), horizontal or vertical | popup, hook, bots |
| STATIONARY | period, on-frames, 20 frames of blinking before it turns on | laser, sparking machine |
| SPAWNER | interval, speed, 30 frames of shadow before a falling object | falling CRT, boss agents |

Contact (any behaviour): `clock_sub(HIT_TIME_PENALTY_S)`, knock Geeko back 16 px, `INVULN_FRAMES` of blinking. Foe movement uses tables and counters, no random numbers, so recorded-input tests stay reproducible.

### 8a.5 The four rooms

Legend: `#` solid, `TT` terminal (2 x 2), `GG` gate (2 x 3), `S` Geeko's start, letters = where a foe starts (d drone, b bug, p popup, c CRT spawner, l laser, s sparking machine, k padlock, h hook, o bot). Column numbers on top. The route is the intended order, not the only one. Clean time is my **guess**, to be replaced by the measured playthrough.

**Room 1: 1990 UNIX Labs** (red brick, CRT glow). 2 terminals, hack 60 frames. Teaches: jump a ledge, hack, avoid one foe.
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
Foes: drone, PATROL on the floor, columns 14-22, slow, hovering at head height (jump over it). Clean time about 25 s.

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

---

## 8b. Boss design (detailed): THE MAINFRAME

Design of the final arena (step M7). Same room engine, same rules as 8a, drafts as in 8a.

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

An agent type belongs to a tier; a **lane** is (tier, side): agents of that tier leave the eye towards that side at head height of that tier. This settles the contradiction in section 5: **a hacked terminal closes its own lane** (tier, side); **both terminals of a tier close the agent type for good**; every terminal also takes 1/6 off the boss bar.

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
* **Tile budget (estimate, tight):** head about 12 + eye 2 frames x 16 = 32 + terminal 8 + cracks about 12 = 64 decor tiles, which is why the decor range is 96 tiles (`0x80-0xBF` and `0xE0-0xFF`).

### 8b.4 Tests

Recorded-input playthrough (fixed spawn table): hack all six, reach `WIN`; lanes close one per terminal (read the lane flags from RAM); an agent hit lowers the clock by the penalty once; the boss bar in the HUD row equals the unhacked count; at most 8 sprites per scanline in the worst scene.

---

## 9. Decisions

Decided (your messages of 2026-10-19): **10 minutes in total on one clock (first said 26, cut to 10 so the clock fits a 16-bit frame counter); a race takes 60 s; a crash or foe contact costs fixed time; each hacked terminal adds time; constants are plain `#define`s in `src/tuning.h`.**

Decided (your messages of 2026-10-12):
* **RACE + HACK** both stay in the game; the runner is the way into each epoch.
* **No combat**; no tongue.
* **No death (2026-10-19):** no lives, no health. A race crash slows Geeko down (reaching 88 MPH takes longer); the only game over is the clock running out.
* **Hack = hold B.**
* **No tail-hang.** Rooms use Mario-style jumping on the hoverboard, with inertia on start and stop (section 4a).

Still open:
1. ~~**Mapper**~~ **Decided 2026-10-19: MMC3** (section 7a).
2. **Music engine:** FamiTone2 (recommended, already in the SDK) vs FamiStudio (section 7b). The **converter** is decided (2026-10-16): FamiStudio exports `.ftm` to FamiTone2 data (section 7b).
3. **4th epoch:** 2020 (brief) vs 2054. Plan assumes 2020, with the Mainframe as the 2054 AI era.
4. **Rooms per epoch:** 1 for the first playable version, more later.
5. **Terminals per room and hack times:** values in section 3 are guesses; so are the physics constants in section 4a.
6. ~~**Geeko in rooms:**~~ **Decided 2026-10-16:** the 16x16 sprite (2x2 tiles) is used in the race and in rooms.
7. **Story screens:** how many and how long; they need a lowercase/punctuation font (currently only `A-Z 0-9` and a few symbols, and `'` / `,` render blank).
8. ~~**Licence of this repo:**~~ **Decided 2026-10-15: GPL-3.0** (`LICENSE`). See section 10.
9. **Branding:** the concept art uses the openSUSE logo and AI-company-named agents. Proposed renames: **PINCHBOT** (orange blocky crab), **WHIRLY** (teal swirl), **TWINKLE** (cyan star), redrawn rather than copied (`concept-art/PROMPTS.md`). Check the openSUSE logo guidelines.

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
* Nothing here is *forced* on us as long as we copy no code or assets. We already ship MIT / public-domain library code (nesdoug, neslib, FamiTone2) in the ROM: keep a credits file with their notices.
That also rules out commercial use.
* If we stay ideas-only, any licence works. MIT would match most of the neighbours;
* The MIT/CC0 projects (`file-fixers`, `witchnwiz`, `nes-starter-kit`, `nesdoug-llvm`) are safe sources if we want a reference implementation to copy from; `from-below` and `famidash` content are not.
