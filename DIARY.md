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

### Geeko sprite, five iterations
Reference: the openSUSE logo (side-view chameleon, spiral tail, big ring eye, long smile).
1. Shrunk the head (too large), then added a small smile (a 1-px mouth gap with the lip tip curled up) and a wiggling tongue as a separate 1-tile sprite.
2. Redrew Geeko as 24x24 in the logo's style; wider hitbox. Result looked like a dog with a big spiral tail.
3. Removed the tail and the orange McFly vest, made the body the main shape. Still wrong.
4. Removed the tongue, added a small curled tail and longer legs.
5. Made the **head** the main feature (casque spike, 6x6 ring eye, long smile). This matched the logo.

What worked: drawing whole sprites as ASCII pictures and slicing them into tiles (`put_sprite()`), plus a Pillow preview rendered from the *built* CHR and the real metasprite tables, so what I looked at was what the game uses. Colour budget is 3 + transparent, so the pupil and mouth are transparent holes (dark sky shows through). Rolling back was only possible from a manual backup (no git yet).

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
