# Development diary

What was built, with which tools, what went wrong and how it was solved. Newest entry last.
Work before 2026-10-12 is not recorded here (the repo had no git history before that day).

## Tools and technologies

| Tool | Version | Used for | Notes / gotchas |
|---|---|---|---|
| **llvm-mos SDK** (`mos-nes-nrom-clang`) | v23.3.0 (pinned in `Containerfile`) | Compiling C for the 6502 / NES | Targets exist for other mappers too (`mos-nes-cnrom-clang`, `-mmc1-`, `-mmc3-`, ...); see plan.md section 7 |
| **neslib** (Shiru) + **nesdoug** (Doug Fraker) | bundled with the SDK (`-lneslib -lnesdoug`) | PPU/OAM helpers, VRAM buffer, pad input, `split()` (sprite-0 scroll split), `check_collision()` | `check_collision()` reads the first 4 bytes of both structs as x, y, width, height |
| **NES hardware** | NROM-256 (mapper 0) | 32 KB PRG + 8 KB CHR-ROM | 256 BG + 256 sprite tiles total; 8 sprites per scanline; raw APU register writes for sound |
| **Python 3** | 3.13 on the host, 3.6 inside the container | `tools/build_chr.py`: ASCII art -> `assets/graphics.chr` | Standard library only. Edit art in the script, not in the `.chr` |
| **Pillow** | 12.3.0 (host only) | Preview renders of sprites while designing | Not needed to build the game |
| **GNU make** | system | `make`, `make test`, `make clean` | `CC` in the Makefile points to a path that does not exist here: pass `CC=` (container) |
| **Podman** | 6.1.2 | Build container (`Containerfile`: `registry.suse.com/bci/bci-base` + llvm-mos tarball) | `podman run --rm -v "$PWD":/src:Z llvm-mos make CC=mos-nes-nrom-clang`. Output was byte-identical to the ROM built with a local toolchain |
| **Mesen2** | 2.1.1 (`~/.local/opt/mesen2/Mesen`) | Headless test runner: Lua scripts press buttons, read RAM by label, log PPU writes, take screenshots | Needs `LD_PRELOAD=/usr/lib64/libstdc++.so.6` and two CLI overrides, see entry below |
| **Lua** | 5.4 (`luac -p` for syntax checks) | `tests/smoke.lua`, `tests/trace.lua` (run inside Mesen2) | A Lua error inside a Mesen callback hangs the runner: always run under `timeout` |
| **Nestopia** | 1.53.2 (flatpak) | Playing the ROM by hand | No scripting |
| **Git** | system | Version control (initialised 2026-10-12) | `*.nes`, `.elf`, `.mlb`, `graphics.chr` are ignored (build outputs) |
| **Gemini** (Google) | - | Concept art in `concept-art/` | The files were named `Gemini_Generated_Image_*` (renamed by content on 2026-10-15); the lower-res JPGs look like earlier renders of the same four images |
| **FamiTone2** + `ft2-text2data` / `ft2-nsf2data` | in the SDK / container ("famitone version 2d") | Music and SFX engine and converters. **Available, not used yet** (plan.md 7b) | Pipeline reproduced against `nesdoug-llvm`: `ft2-text2data song.txt -ca65`, then `sed 's/@/.L/g'` |
| **Claude Code** (Anthropic CLI) | - | AI pair programming: code, analysis, tests, docs | |

Files that carry the knowledge: `AGENTS.md` (formerly `CLAUDE.md`; how to build/test, architecture, gotchas), `plan.md` (game concept and roadmap), this diary (history and reasons).

---

## 2026-10-12

### Starting point
NROM runner (jump over bugs and tumbleweeds, duck under drones, 20 -> 88 MPH), a 16x24 Geeko, everything in one `src/main.c` (542 lines), no tests, no git, ROM built with a local llvm-mos that was no longer installed.

### Build in a container
`Containerfile` downloads the llvm-mos SDK release into `bci-base`. The ROM it builds is byte-identical to the one that was already in the tree, so the container is a drop-in toolchain. The SDK tarball was first downloaded unverified; since 2026-10-15 the `Containerfile` pins its SHA256 (see "Publishing" below).

### Geeko sprite, six iterations
Reference: the openSUSE logo (side-view chameleon, spiral tail, big ring eye, long smile).
1. Shrunk the head (too large), then added a small smile (a 1-px mouth gap with the lip tip curled up) and a wiggling tongue as a separate 1-tile sprite.
2. Redrew Geeko as 24x24 in the logo's style; wider hitbox. Result looked like a dog with a big spiral tail.
3. Removed the tail and the orange McFly vest, made the body the main shape. Still wrong.
4. Removed the tongue, added a small curled tail and longer legs.
5. Made the **head** the main feature (casque spike, 6x6 ring eye, long smile). This matched the logo.

What worked: drawing whole sprites as ASCII pictures and slicing them into tiles (`put_sprite()`), plus a Pillow preview rendered from the *built* CHR and the real metasprite tables, so what I looked at was what the game uses. Colour budget is 3 + transparent, so the pupil and mouth are transparent holes (dark sky shows through). Rolling back was only possible from a manual backup (no git yet).

6. **2026-10-16: redesigned as 16x16.** References from the user: a round-headed cartoon chameleon on a branch (rejected as too detailed for 16 px), a shaded pixel-art chameleon, and a very simple striped one with a spiral tail (this one worked). Result: striped dome back (even columns light green, odd dark green), big white eye with a dark pupil, small curled tail, 2-wide legs, standing on a white hoverboard with a dark-green stripe; crouched pose = body squashed onto the same board. 16x16 = 2x2 tiles instead of 3x3: 2 sprites per scanline instead of 3. Palette 0 changed from green/orange/white to light green/dark green/white, so the orange is gone from Geeko (the fire trail keeps orange/yellow). `GROUND_Y` 148 -> 155 so the board stays at the same height; hitbox x+2, w 12; `make test` passes (the drone XFAIL is unchanged). Looked at in the real emulator with a throwaway Lua script (screenshots of ride, duck, jump and the 88 MPH trail).

### Mesen2 for automated testing
Goal: play the game without a human. Nestopia has no scripting; Mesen2 has Lua and a headless runner.
* Installed the Linux x64 zip into `~/.local/opt/mesen2`.
* **Crash on start: `std::bad_cast`.** gdb backtrace: a static initialiser in `MesenCore.so` compiles a `std::regex`, which asks its *statically linked* libstdc++ for a locale facet and fails next to the system's GCC 16 libstdc++. `LD_PRELOAD=/usr/lib64/libstdc++.so.6` fixes it. Locale variables did nothing.
* **Input did nothing.** The fresh headless config has controller port 1 set to "None". Fixed with `--nes.port1.type=NesController`.
* **Screenshots/file access hung the runner.** Lua `io` is disabled by default. Fixed with `--debug.scriptWindow.allowIoOsAccess=true`.
* Mesen's working directory is not the caller's: the test gets the project root through an environment variable.
* RAM addresses come from `hacktothefuture.mlb`, which the linker writes, so tests look variables up by name.

### Smoke test (`make test`)
`tests/smoke.lua` boots, starts, ducks, jumps, clears and hits obstacles, restarts, and lets an idle player crash. I checked that it can fail by breaking collision in a temporary copy (3 checks failed) and by slowing a frame down (4 failed). It also found a real bug: a standing player is never hit by a drone (`XFAIL`, so the run still passes).

### Scripted emulator development, and the tests it enables
The emulator is part of the development loop, not only a place to look at the game. Mesen2 runs the ROM with no window and executes a Lua script that is called back every frame. From that script I can:

| Capability | API used | Used for |
|---|---|---|
| Press buttons on any frame | `emu.setInput({start=true}, 0)` inside an `inputPolled` callback | Playing: start, jump, duck, restart |
| Read and write RAM by name | `emu.read` / `emu.write`, addresses looked up in `hacktothefuture.mlb` | Asserting game state; **forcing** state (plant an obstacle, stop random spawns, set `mph`) |
| Watch hardware writes with timing | `emu.addMemoryCallback(..., write, 0x2005)` + `emu.getState()["ppu.scanline"]` | Finding and then guarding the HUD split scanline |
| Capture the picture | `emu.takeScreenshot()` | Review images in `tests/out/`; per-frame hashes |
| Stop with a result | `emu.stop(code)` | `make test` exit code (CI-friendly) |

Pattern in `tests/smoke.lua`: the test is a **coroutine that yields once per frame**, so it reads like a script (`wait(30)`, `press({a=true}, 1)`, `wait_until(fn, max)`), with `check()` for hard failures, `xfail()` for known bugs (reported, not failing), a frame watchdog, and a `done` flag because `emu.stop()` is asynchronous.

Tests this already enables (all in `make test` unless noted):
* **Behaviour:** boot to title, Start, speed starts at 20 MPH, duck on and off, jump and land, score grows, jump clears a ground obstacle, a standing player crashes into one, restart resets the score, an idle player crashes naturally.
* **Deterministic collisions:** instead of waiting for random spawns, the test writes an obstacle straight into RAM next to the player, then checks the outcome two frames later.
* **Hardware timing:** the scroll split must land on scanline 23 (found: 21 on the old build). A frame budget check: the scroll advances exactly `speed_px` every frame, which fails if a frame overruns vblank.
* **Known bugs as tests:** the drone that never hits a standing player is an `XFAIL` that flips to a plain check when fixed.
* **Refactor safety net** (`tests/trace.lua`, run by hand): 900 frames of scripted input, one hash per rendered frame; run it before and after a change and `diff` (identical = same video). Used for the module split.
* **Mutation checks** (done by hand in a temporary copy): break collision, or slow a frame down, and confirm the right tests fail. Without this a green test proves little.

Tests it makes possible next (see `plan.md`): the warp sequence by poking `mph = 88`; hold-B hack timing and the exit gate; the room physics numbers (measure the speed ramp and the stopping distance of the inertia from RAM); lives and game over per epoch; the boss and its six terminals; the CHR bank swap per epoch (screenshot or PPU memory compare); the 8-sprites-per-scanline overflow; audio (log writes to `$4000-$4017` to assert that an effect fires); a soak test with random input that checks the state never becomes invalid; recorded-input playthroughs as regression tests for whole levels.

Rules learned: always run under `timeout` (a Lua error in a callback hangs the runner); pass absolute paths (Mesen's working directory is not yours); the emulator is deterministic, but one screenshot hash in five runs differed in a single frame, so hash-based checks need a retry before anyone calls them failures.

### HUD bug: garbled bottom of the second text row
Method: a memory-write callback on `$2005` (PPUSCROLL) that records `ppu.scanline` for every write. Result: the scroll split landed on scanline 21 on every frame, but the HUD text row covers scanlines 16-23, so the bottom stroke of each glyph was scrolled. Fix: sprite 0 moved down and given its own 1-pixel tile (`0x23`), with a matching 1-pixel background tile (`0x89`) on the HUD's last scanline, so the hit is on scanline 23. A regression check in the smoke test asserts the split scanline and fails on the old commit ("found: 21").

### Git
`git init`, `.gitignore` for build outputs and `tests/out/`, initial commit `9f2ed27`. Later changes are left uncommitted for review.

### Concept art and plan
Four concept images (box art, 1990 stage, 2010 stage, boss) are described in `plan.md` with what each implies for the design. The new design: at 88 MPH Geeko time-travels into single-screen epochs (1990 / 2000 / 2010 / 2020), hacks every terminal there by standing in front of it, avoids era-themed foes, and finally hacks the 6 terminals of the Mainframe boss. Biggest technical consequence: NROM's 8 KB CHR cannot hold per-era art, so the plan proposes a mapper with CHR banking (CNROM first). Also logged in the plan: the score HUD shows score x 10 (a column off by one), `warp_flash_timer` is set but never used.

### Decision: race + hack
The game is both: the existing **race** (runner, 20 -> 88 MPH) and a **hack** half (rooms with hold-to-hack terminals, an exit gate, a six-terminal Mainframe boss, story screens), described in `plan.md`: what is built, how it adapts to NES limits, and what is deliberately left out (many levels, secret levels, stomp mechanics, a tail-hang).

### Research: mapper, audio, licences, and the next steps
* **Mapper alternatives** in the current llvm-mos SDK (read from its linker scripts): NROM (now), CNROM (8 KiB CHR banks, `set_chr_bank`), MMC1 (4 KiB CHR banks, PRG banking), MMC3 (1 KiB CHR banks and a scanline IRQ), UNROM, UNROM-512, GTROM, Action53. Recommendation: CNROM first. Table in `plan.md` 7a.
* **Audio:** the SDK already ships FamiTone2 (`music_play`, `sfx_play`, DPCM) and its converters. I ran the converter in the container on a song from the `nesdoug-llvm` sibling and, after one `sed` (`@` to `.L` local labels for the LLVM assembler), got byte-identical output, so the whole pipeline works with our compiler. The other NES projects in the folder use FamiTone2 / FamiTone 5 (`from-below`, `file-fixers`, `mbh-A53-witchnwiz`), FamiStudio (`famidash`) or a custom FamiTracker driver (`nes-starter-kit`). `plan.md` 7b.
* **Licences** of those projects: MIT (`nesdoug-llvm`, `file-fixers`, `mbh-A53-witchnwiz`, `famidash` code, `nes-starter-kit`), public domain (`neslib`), LGPL-3.0 (`MK1_NES`), none for the game in `from-below`. `plan.md` 10.
* **Design decisions** from the user: hold B to hack; no tail-hang, Mario-style jumping with inertia on the hoverboard instead (plan.md 4a); race and hack both stay. The four known bugs (score x 10, drone, warp only a sound, README) are in plan.md section 0 with a fix and a test for each (Phase 0).
* `concept-art/PROMPTS.md` holds image prompts for an NES box, cartridge and manual that show the merged concept.

### Modular source
`main.c` (monolith) split into `player`, `obstacles`, `progress`, `render`, `hud`, `screens`, `sfx` plus `game.h`; `main.c` keeps only the state machine. Each module has a header with what others may use and keeps its state private. The Makefile compiles `src/*.c`, so new modules need no Makefile edit.

How it was verified without trusting the eye: `tests/trace.lua` plays a fixed 900-frame input script and hashes every rendered frame. The refactored ROM matched the pre-refactor ROM frame for frame in 4 of 5 runs; one run differed in a single frame (216) and could not be reproduced (probably a screenshot timing race in the harness; the same ROM was identical in the other runs). `make test` passes, and a new check ("one main-loop iteration per video frame": scroll advances exactly `speed_px` per frame) guards against the extra function calls pushing a frame over its budget. That check failed as expected when a frame was deliberately slowed down.

### Memory and commit
Session memory (decisions, sibling-project references, feedback) lives in `memory/` inside the repo, with a symlink from Claude Code's memory directory so it still loads. The first session's work was committed in three commits (code, docs, concept art). `concept-art/` grew from 8 to 11 Gemini images during the day; the 7 new ones are not catalogued yet.

### Publishing (2026-10-15)
Pushed to a public GitHub repo, `ilmanzo/hw26-hack-to-the-future`. Binary images are gitignored; the concept art is linked from an external site, and the images-only commit was removed from local history before the first push so the 31 MB never reached GitHub. The repo is **GPL-3.0** (`LICENSE`). The README was rewritten to match the current game. The `Containerfile` now downloads the llvm-mos SDK to a file and checks its SHA256 (`LLVM_MOS_SHA256`, matches GitHub's release asset digest and a local download); a wrong hash fails the build (tested), and the ROM built in the pinned container is byte-identical to the previous one.

### Open / next
See `plan.md` section 9 (open decisions) and section 0 (known issues): score HUD column, drone hitbox, sprite-per-scanline overflow, mapper choice.

### Crash noise never stopped (2026-10-16)
Reported by the user: after a crash the sound kept playing, also after restarting. Reproduced in Mesen (`emu.getState()`): noise length counter stuck at 18 with `halt=true`, 7 s later and after restart. Cause: `sfx_crash()` wrote `$400C = $3f`, bit 5 is the length-counter halt. Fix: `$400C = $0f` (decay envelope, ~1 s fade) and `$400F = $c0` (length 192, 1.6 s). Regression check in `tests/smoke.lua` ("crash noise stops by itself"); it failed on the old ROM and passes now. The other four effects were checked by reading their register values (none sets the halt bit), not by running them.

### Music export pipeline (2026-10-16)
FamiStudio 4.5.3 (container, Xvfb) exports CC0 `.ftm` files to FamiTone2 data; `nes_23-chill` was then played by the SDK's FamiTone2 in a throwaway ROM and the APU registers checked in Mesen (details and caveats in `plan.md` section 7b). `ft2-text2data` rejects these files, FamiStudio's own FamiTone2 export does not. Nothing of this is in the game yet (Phase 2).

### Music and effects in the game, draft (2026-10-16)
FamiTone2 now drives everything: `sfx.c` is `music_play` / `sfx_play`, the raw APU pokes are gone. `assets/audio/chill.ftm` (CC0, Free NES Music Pack) and `sfx.txt` (five effects as FamiStudio text songs, written by script) are exported by `tools/export_audio.sh` (FamiStudio in a container) to `chill.s` / `sfx.s`; the Makefile links `-lfamitone2`. Noise note C4 in FamiStudio = noise period index 14 (the old effect's `$400E = $0e`); C2 = 6, C#3 = 9, C3 = 10. FamiStudio's noise SFX volume bytes have the length-counter halt bit set (`$3f ... $31`), harmless here because the music engine rewrites `$400C` every frame once the effect ends. PRG used: about 8.6 KB of 32 KB. Tests: the old `crash noise` check (length counter) could no longer apply, so the audio checks now read the emulated APU state (`sounding()` in `tests/smoke.lua`: length counter running and volume above 0); on the pre-music ROM from `git HEAD` they fail on the crash noise as they should. A first version based on register writes could not see a held note, so I dropped it.

### Phase 0 bug fixes (2026-10-19)
Fixed the known issues of `plan.md` section 0, each with a check in `tests/smoke.lua`; all of them fail on the pre-fix ROM and pass now (mutation check: stashed `src/` + `tools/`, rebuilt, ran the new tests, got 3 FAILs).
* **Score HUD:** `hud_update()` wrote the digits at column 15, the text has them at 16-20 (`SCR:` ends at 15), so the screen read `SCR000460`. Now column 16. Test: read nametable A row 2 through `nesPpuMemory` and compare the whole row with `mph`, `score`, `hi_score`.
* **Drone:** flew at y 136 (box 138-147), above the standing hitbox (150+), so nothing ever hit it. Now y 147 (box 149-158): hits a standing player (156+), passes over a ducking one (161+). Test lets the game spawn a drone (type is random: spawn until one appears), moves it onto Geeko keeping the y the game chose, then asserts crash when standing and no crash when ducking. A first version planted its own y and passed on the old ROM too: it tested the test, not the game.
* **Font:** added `'` and `,`. Test: every title-screen tile in 0x21..0x5F must have a non-blank pattern (the generic check also covers any future string).
* **Sprites per scanline:** counted from OAM (`nesSpriteRam`), not from the PPU overflow flag (its hardware bug gives false hits). Worst case playing at 88 MPH (2 obstacles + Geeko + both fire-trail flames) = exactly 8, no headroom; the test asserts <= 8. Crashed at 88 MPH with 2 obstacles the star makes it 9: the PPU drops the last sprite in OAM order (the trail's second flame), cosmetic. Setting 88 MPH in a test: `mph` is recomputed from `score` every 4 frames (`20 + score/40`), so poke `score` (2720), not `mph`.
* **Makefile:** new `make podman` target (container build). `CC` itself is unchanged: it stays the hardcoded path, overridable on the command line.
* **Not done:** the 88 MPH warp (needs the new state, Phase 1).

### MMC3 spike and step-by-step plan (2026-10-19)
Question: can an MMC3 IRQ replace the sprite-0 split, and is another mapper more convenient? Checked in the SDK (`mos-nes-*-clang`: nrom, cnrom, mmc1, mmc3, unrom, unrom-512, gtrom, action53; only `nes-mmc3/include/mapper.h` has IRQ functions) and with a throwaway copy of the game in `/tmp/mmc3-spike` (not in the repo): built with `mos-nes-mmc3-clang`, ~30 lines changed, `make test` passes unchanged. Details and numbers in `plan.md` 7a: IRQ writes land on scanline 23 with count 22 (24 with 23), game logic starts at scanline 3 instead of 29 (probes: write callback on `scroll_x`, read callback on `$2002`), default ROM is 768 KB until `MAPPER_PRG_ROM_KB` / `MAPPER_CHR_ROM_KB` are set (then 73,744 bytes). Wrong first guesses worth remembering: the SDK's `nesdoug-llvm/33_MMC3/crt0.s` is a leftover (not used by its CMake build), so it says nothing about how the IRQ array restarts; `mph` poking in tests does not stick (see Phase 0 entry). User decision: stay with MMC3. `plan.md` section 8 rewritten as milestones M1-M8 with test-able steps, budgets, open decisions D1-D5 and cut lines A-D.

### Design decision: no death, game over only by the clock (2026-10-19)
User: a crash in the race should only slow Geeko down (more time to reach 88 MPH); the game must never "kill" the player, the only game over is time expiring. `plan.md` updated: section 1 (loop rules), 4, 6 (HUD `TIME` instead of `LIVES`), 8 (steps 2.3 race without death, 2.4 clock, 5.2 contact costs time), 9 and the decisions table (D1 decided; D6 clock scope, D7 contact penalty, D8 score meaning added). Numbers that matter for tuning, from the code: `mph = 20 + score/40` with a score tick every 4 frames means +1 MPH per 160 frames and 181 s to reach 88 MPH with no crash today; a 10-MPH loss would cost 27 s at that rate (about 9 s if the clean race is retuned to 60 s). The current smoke checks that expect `STATE_GAMEOVER` after a crash (ground obstacle, drone, idle, silence after the crash) have to be rewritten in step 2.3. Nothing in the code has changed yet.

### Time rules, tuning.h, room and boss design (2026-10-19)
User decisions: 26 minutes in total on one clock (Hack Week 26); a race takes 60 s; a crash or foe contact costs fixed time (a race crash also slows Geeko); each hacked terminal adds time; the constants are plain `#define`s. Done: `src/tuning.h` (clock 1560 s, cap, penalty 5 s, bonus 10 s, 53 frames per MPH, crash loss 10 MPH, invulnerability 60 frames, hack frames per era); syntax-checked in the container, not included by any source yet (first use: plan steps 2.3 and 2.4). `plan.md`: new sections 8a (room design: screen anatomy, jump envelope, tile classes, foe behaviours, four room maps, time economy) and 8b (boss arena, agents and lanes, overload, tests), steps 4.1a (room generator) and 7.x updated.
Findings worth keeping: (1) **26 minutes = 93,600 frames, which does not fit a 16-bit counter**: the clock must be seconds plus a 0-59 frame counter. (2) The 4a physics, simulated in the same fixed-point order as `player.c`, give an apex of 49.2 px (6.1 tiles) and 43 frames of air time on the level, not the 51.6 px of the continuous formula; layout rules (rise at most 4 tiles, gaps 4/3/2 tiles) come from that table; not yet measured in the emulator. (3) The maps in 8a/8b were checked by a throwaway script (`/tmp/roomtool/`, not in the repo): all terminals and gates reachable under those rules; a first version of the script wrongly treated every platform above the floor as unreachable (it ignored approaching from the side), fixed. (4) Plan section 5 contradicted itself (a hacked terminal removes a lane vs a hacked pair removes the agent type); resolved with lane = (tier, side). (5) With one room per epoch a flawless run needs about 4:30 of the 26:00 (guessed room times), so the clock would almost never end a run: open question D4/D9.

### Clock cut to 10 minutes, room checker saved (2026-10-19)
User: total time 10 minutes (was 26) so the clock fits a 16-bit counter. 600 s = 36,000 frames; the limit is 65,535 frames = 1,092 s, so `time_left` is a plain u16 frame counter again (the seconds-plus-sub-counter design of the previous entry is dropped). `src/tuning.h`: `GAME_TIME_S` 600, `TIME_MAX_S` 600, and an `#error` if either goes above 1092 (checked: 1100 fails the compile in the container). Economy with the guessed room times (plan 8a.6): a flawless run nets 4:30 and ends with 5:30 left; a first-time player at twice the room and boss times ends with about 2:00 left. The 26-minute figure had made the clock meaningless (21:30 spare).
`tools/check_rooms.py` saved in the repo (it was `/tmp/roomtool`): the five maps of plan 8a/8b as rectangle lists, reachability check, `-v` prints the maps, exit 1 on failure. Changes from the throwaway: the jump limits are derived from `GRAVITY` and `JUMP_IMPULSE` read out of `src/player.c` (they reproduce the table of 8a.2: gaps 4,4,3,3,3,2 and max rise 5), a self-test checks that possible rooms pass and impossible ones fail (rise 6, gap 5), and the ASCII maps in plan.md were regenerated from it because the first copies were shifted one column against their ruler. A mutation test (shelf gap 6 with no other route) fails as it should; my first mutation passed because the floor reached the platform through a third shelf, which was a bad test, not a checker bug.

### Game-over screen (2026-10-19)
User: a game over must show "GAME OVER, PRESS A TO RESTART". `crash_banner_show()` is now `game_over_show()` (`screens.c`): `GAME OVER` at (11, 9) and `PRESS A TO RESTART` at (7, 11) through the VRAM buffer, over the frozen playfield; the HUD rows still show the final numbers. A and Start both restart (`state_gameover` already accepted both). Checks in `tests/smoke.lua`: the two strings read from nametable A, and the restart after game over now uses A. Today this screen still comes from a crash; the clock (plan 2.4) will only change the trigger. Uncommitted.

### Race without death, score, clock, warp and a playable room 1 (2026-10-19)
Built in one session, all on NROM, all covered by `make test` (`tests/smoke.lua`, `tests/room.lua`, helpers in `tests/lib.lua`):
* **Race:** `mph` is its own variable (+1 per 53 frames, 60 s from 20 to 88), a crash costs 10 MPH, 5 s and 60 frames of invulnerability (Geeko blinks), the music keeps playing. `invuln` counts down in `clock_tick()`.
* **Clock and game over:** `time_left` (u16 frames, 600 s). At 0: the game-over screen (GAME OVER, score, rank, PRESS A TO RESTART), drawn with rendering off.
* **Score:** speed points, dodge streak multiplier (an obstacle that passes untouched), terminals, clean room, ranks. A clean race measured 63 dodges and 4,907 points; the flawless-game model in the smoke test gives 25,978 for the 26,000 target (`CLEAN_ROOM_POINTS` was moved from 300 to 250 to centre it).
* **Warp and room:** 62 frames, the HUD year rolls, the room is drawn at the flash peak. `hero.c` (inertia, jump, per-axis tile collision, knockback), `room.c` (rectangles drawn at load, terminals with a 4-tile hack bar, gate, drone patrol), room data generated into `assets/rooms.h` by `tools/build_rooms.py` from `tools/check_rooms.py`. Measured: 23 frames to full speed, 33 frames and 31 px to stop, apex 49.2 px. A bot plays the whole room (10.5 s, no hits, 450 points).
* **Bugs found by the tests:** (1) a garbled HUD digit when the game-over frame queued about 120 bytes of VRAM buffer (the NMI flushes about 100 bytes per vblank): fixed by writing only changed HUD fields and drawing the big screens with rendering off. (2) A jump from 2 px left of a platform bumped its underside (the box overlapped the platform's first column): the bot now starts from x 4. (3) My first drone test planted its own y and passed on the old ROM too; the test now lets the game spawn the drone. (4) My first bot route jumped over the drone at the wrong moment; it now takes the shelves.
* **Docs:** `plan.md` rewritten to match the code (status, as-built mechanics, roadmap with a State column, designs of rooms 2-4 and the boss kept as design); all references to another platform's game were removed from the project (docs, memory, a prompt file, an image file name, code comments). One commit message in the history (`27f6cb2`) still mentions it; rewriting published history was not done.

### Shared Geeko, time map, blue sky, MMC3 port, sprites measured (2026-10-19)
* **One Geeko body:** `geeko.c` holds jump, fall, run, hitbox, blink and drawing for a `struct Geeko`; `player.c` (race) and `hero.c` (rooms) add only their ground. The race therefore got the variable-height jump (a held jump 49.2 px in both, a tap 13 px). The label file lists a struct field by field and the compiler drops unused fields (the race has no `vx`, no facing), so the tests read fields by label (`T.geeko`); a one-element-used array shows up as `name.0`.
* **Time map:** `map.c` + `gfx.c`; tests in `tests/map.lua`. The first version of the map test failed because it checked the state right after the A press while the transition (`race_begin` waits for NMI) takes a few frames: a wait, not a bug.
* **Blue sky** for the race (`C_SKY_BLUE` 0x11), black rooms; the test reads palette RAM.
* **MMC3 port** (steps 1.1-1.3): one real problem on the way: I cut `render_map_sprites` out of `render.c` with a careless slice (the link error showed it). All tests passed unchanged, including the split on scanline 23. ROM 73,744 bytes.
* **Sprites per scanline** (`tests/sprites.lua`): race 6, drone over a ground obstacle 6, crash burst 7, warp with the fire trail 8, room 4, map 2. The earlier "9 while crashed at 88 MPH" no longer exists: the race ends at 88 MPH (the trail only appears in the warp) and the crash burst has no trail.
* **Commits** split into logical groups (race/clock/score, room art and tooling, room engine, docs, shared Geeko, blue sky, map, MMC3, sprites test). To split work that was already written, the Stage A versions of the mixed files were reconstructed, built and tested in a temporary worktree before committing.

### Cheats, a stale HUD write, a broken clean build, and the history scrub (2026-10-19)
* **Cheats** (`tuning.h`, switched before the build, both 0 in a release): `DEV_LEVEL_SELECT` (map: LEFT/RIGHT pick a level, A its race, SELECT its room; replaces the title-screen SELECT) and `DEV_INVULNERABLE` (`invuln` never reaches 0 and stays under the crash-pose threshold, so Geeko blinks without the burst). `tests/cheats.lua` follows the build; other scripts use `T.needs`. Checked with all four combinations of the switches.
* **Found by the tests:** the map timer is a byte and wrapped after 255 frames (only visible with the cheat, which waits as long as it likes): it saturates now. A HUD number queued in the last frame of the map was flushed after the room had been drawn (`HACK20/2`): `gfx_screen_off()` flushes the queue before a whole-screen redraw.
* **Found by building a commit in a clean worktree:** `check_rooms.py` still read the jump constants from `player.c` after the Geeko refactor, so a clean checkout could not build (the generated `rooms.h` hid it). Fixed in its own commit.
* **History scrub:** the history was rewritten twice and force-pushed (`main` and the `concept-art` tag, which carries the GitHub release "Concept art" with its 10 images: moved to the equivalent rewritten commit, release and assets intact). Every commit message and every file version in `main` is free of the word now (checked over all diffs); the final tree is byte-identical. The scrub is a text transform (`/tmp/scrub.py`, not kept): it removed the section about the other platform's game, parentheticals and sentences that mention it, and kept the rest. GitHub still serves the old commits by hash until it garbage-collects them (a support request can purge them), and anyone who cloned before has the old history. A local backup ref `refs/original/refs/heads/main` and the reflog still hold the old history; deleting them (`git update-ref -d` and `git reflog expire` + `git gc --prune=now`) was left to the user.
