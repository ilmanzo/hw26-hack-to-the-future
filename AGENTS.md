# AGENTS.md

This file provides guidance to AI coding agents and human contributors working in this repository.

## Project

NES endless runner ("Hack to the Future: Geeko McFly", openSUSE Hack Week 26). Direction: `plan.md` (race to 88 MPH + hack terminals in era rooms, ported from the MSX sibling). History and tool notes: `DIARY.md`. Session memory notes (decisions, references, feedback): `memory/MEMORY.md`. NROM-256 (mapper 0, 32KB PRG + 8KB CHR), C via `llvm-mos` + `neslib` + `nesdoug`. No tests, no linter, no git repo.

## Build

```bash
make                # builds hacktothefuture.nes (regenerates assets/graphics.chr first if build_chr.py changed)
make clean          # removes .nes and assets/graphics.chr only (not .nes.elf / .mlb)
make CC=<path>      # override compiler
make test           # headless smoke test in Mesen2 (needs a built ROM, see below); exit 0 = pass
```

- `CC` is hardcoded to `/home/andrea/.local/llvm-mos/bin/mos-nes-nrom-clang`. That path does not exist on this machine; build via the container instead (below) or override `CC`.
- Link step also emits `hacktothefuture.nes.elf` and `hacktothefuture.mlb` (Mesen label file).

### Container build (no local llvm-mos)

`Containerfile` installs llvm-mos SDK (pinned `LLVM_MOS_VERSION`, default v23.3.0) on `bci-base`. Verified: output ROM is byte-identical to the checked-in `hacktothefuture.nes`.

```bash
podman build -t llvm-mos -f Containerfile .
podman run --rm -v "$PWD":/src:Z llvm-mos make CC=mos-nes-nrom-clang
```

The SDK tarball is verified against `LLVM_MOS_SHA256` (sha256 of `llvm-mos-linux.tar.xz`); a mismatch fails the build. When bumping `LLVM_MOS_VERSION`, update the hash too (GitHub shows it as the release asset digest, `gh api repos/llvm-mos/llvm-mos-sdk/releases/tags/<ver> --jq '.assets[].digest'`). The base image is `bci-base:latest`, not pinned.

### Headless testing (Mesen2)

Mesen2 2.1.1 is installed at `~/.local/opt/mesen2/Mesen` (Lua scripting, scriptable input, RAM reads, screenshots). `make test` runs `tests/smoke.lua` (boot, Start, duck, jump, collisions, game over, restart, split scanline; screenshots go to `tests/out/`, gitignored). `XFAIL` lines are known bugs, they don't fail the run. Raw invocation:

```bash
LD_PRELOAD=/usr/lib64/libstdc++.so.6 ~/.local/opt/mesen2/Mesen --testRunner \
  --nes.port1.type=NesController --debug.scriptWindow.allowIoOsAccess=true \
  hacktothefuture.nes script.lua
```

- `LD_PRELOAD` is required on this system: `MesenCore.so` statically links libstdc++ and aborts with `std::bad_cast` at load without it.
- `--nes.port1.type=NesController` is required: the fresh headless config has no controller attached, so `emu.setInput` does nothing.
- `--debug.scriptWindow.allowIoOsAccess=true` is only needed for `io.open` (e.g. saving `emu.takeScreenshot()`). A Lua error in a callback hangs the runner; wrap runs in `timeout`.
- `print()` goes to stdout; `emu.stop(code)` sets the exit code. Set input inside an `emu.eventType.inputPolled` callback; do frame logic in `endFrame`.
- Mesen's working directory is not yours: pass absolute paths (the smoke test takes the project root in the `ROOT` env var).
- Mid-frame PPU timing is observable: `emu.addMemoryCallback(fn, emu.callbackType.write, 0x2005, 0x2005)` plus `emu.getState()["ppu.scanline"]` (that is how the split position is tested).
- RAM addresses come from `hacktothefuture.mlb` (e.g. `game_state` = `$36`, `score` = `$40-41`); read with `emu.read(addr, emu.memType.nesInternalRam)`.

### Run

Interactive emulator: Nestopia (flatpak, has read-only `home` access):

```bash
flatpak run ca._0ldsk00l.Nestopia "$PWD/hacktothefuture.nes"
```
- `hacktothefuture.nes`, `.elf`, `.mlb`, `assets/graphics.chr`, `geeko_test.png`, `tools/__pycache__` are artifacts, not sources.

## Architecture

Three pieces must stay in sync; changing one usually means touching the others:

1. **`tools/build_chr.py`** generates `assets/graphics.chr` (exactly 8192 bytes, asserted). All art is ASCII art in the script (`' '`/`'.'`=color 0 transparent, `1`/`2`/`3`=colors). Layout:
   - Pattern table 0 (`$0000`, BG): tile index == ASCII code for font (0x20–0x5F), road/sky/mountain tiles at 0x80–0x88, sprite-0 target 0x89.
   - Pattern table 1 (`$1000`, sprites): Geeko ride 0x00–0x03 (16x16, 2x2 tiles, row-major) and duck 0x04–0x07 (same 16x16 cell, body squashed onto the board), obstacles 0x10–0x1B, FX 0x20–0x21 (fire trail, crash star), sprite-0 pixel 0x23. Geeko is drawn as two whole ASCII pictures (`GEEKO_RIDE`, `GEEKO_DUCK`) that `put_sprite()` slices into tiles. Edit the picture, not tiles; `sprites.h` assumes this 2-wide row-major layout. Geeko uses sprite sub-palette 0 (light green, dark green, white); the board is in rows 10-15 of both poses.
   2. **`assets/chr-rom.s`** `.incbin`s `graphics.chr` into the `.chr_rom` section. The Makefile finds it via `-Iassets`. **`assets/audio.s`** includes the generated FamiTone2 data (`assets/audio/chill.s`, `sfx.s`) as `music_data` / `sounds_data`.
3. **`src/`** consumes those tile indices by number. One module per concern (`.h` = what other modules may use, `.c` = private state):

   | Module | Owns |
   |---|---|
   | `main.c` | State machine (`STATE_TITLE → PLAYING → GAMEOVER → PLAYING`), `session_start()`, the `while(1)` frame loop. `bank_spr(1)` / `bank_bg(0)` match the CHR layout above |
   | `player` | Geeko input, jump physics (8.8 fixed point), hitbox, drawing. State is private; others use `player_y()` / `player_get_box()` |
   | `obstacles` | Spawning, movement, collision against a `struct Box`, drawing |
   | `progress` | `scroll_x`, `speed_px`, `mph` (20 → 88), `score`, `hi_score`; the 88 MPH warp trigger. Only `progress.c` writes them |
   | `render` | `render_all_sprites()`: the one place that decides OAM draw order |
   | `hud` | HUD text and numbers, sprite 0 + its target tile, the scroll split (`hud_scroll()`) |
   | `screens` | Background nametable, title screen, CRASH banner |
   | `sfx` | FamiTone2 music (`sfx_music_start/stop`) and sound effects (`sfx_jump`, ...) |
   | `game.h` | Constants and `struct Box` shared by several modules |
   | `sprites.h`, `palettes.h` | Data: metasprite arrays (`dx, dy, tile, attr`, `128`-terminated) and palettes. Sprite attr palette index selects the sub-palette (0 Geeko, 1 tumbleweed, 2 bug/drone, 3 FX) |

   The Makefile compiles every `src/*.c`; a new module needs no Makefile change. Cross-module calls are not inlined (no LTO); the main loop still fits in a frame (`make test` checks that).

### Runtime model

- Frame loop: `ppu_wait_nmi()` → `hud_scroll()` (blocks until the sprite-0 hit while playing) → read pad → one state function → `render_all_sprites()`.
- **Sprite-0 hit split** (`hud.c`): stationary HUD on nametable rows 0–2, scrolling playfield below. Sprite 0 (`hud_sprite_zero()`: `oam_spr(240,22,0x23,0x20)`, behind BG) must overlap the single pixel of BG tile `0x89` placed at `NTADR_A(30,2)` (`hud_draw_split_target()`, also called by the title screen). The hit lands on scanline 23 (last HUD scanline) so the scroll applies from 24; the smoke test asserts this. A hit earlier than the HUD's last scanline scrolls the bottom strokes of the HUD text (this bug existed with the old sprite 0 at Y=20 / tile 0x80: hit on scanline 21). Moving either piece, or changing those two pixels, breaks the split. `split(scroll_x)` runs only in `STATE_PLAYING`; other states force scroll 0. OAM slots 0–3 are reserved (`oam_set(4)`) for sprite 0.
- Both nametables A and B are filled by `background_draw()` for seamless horizontal scroll; only A gets the HUD text (HUD is above the split so B never shows it).
- HUD numbers update through the nesdoug VRAM buffer (`multi_vram_buffer_horz`), not direct PPU writes, while rendering is on. Direct `vram_*` writes require `ppu_off()`.
- Speed derives from `mph` (20→88, 88 triggers warp); `speed_px = 2 + mph/25`.
- OAM draw order matters (`render.c`): sprite 0 first, then obstacles, Geeko, fire trail. Past 8 sprites per scanline the PPU drops the lowest-priority ones; Geeko is 2 tiles wide, but keep the fire trail last.
- Collision: `nesdoug check_collision()` takes pointers to structs whose first four bytes are `x, y, width, height`. `struct Box` (game.h) and `struct Obstacle` (obstacles.c) keep those first on purpose; do not reorder.
- Geeko-art-dependent hand-tuned numbers: hitbox (`player.c`: x+2, w 12; standing y+1, h 15; crouched y+6, h 10), crash star at x+4, y+2, fire trail at y+7 / y+8 (`render.c`), and `GROUND_Y` 155 (`game.h`, sprite top; the board's bottom row sits at y 170 as before). Both poses keep the board in the same rows so it does not jump when ducking. Re-check these if you redraw Geeko.
- Sound: FamiTone2 (in neslib's NMI). Music = one track (`assets/audio/chill.ftm`, CC0, placeholder) that plays while `STATE_PLAYING` and stops on the crash; the title screen is silent. Effects use slots 0-3 by APU channel: Pulse1 jump/duck, Pulse2 beep, Triangle warp, Noise crash; FamiTone2 has no volume column and few effects, so the song was cut down on export (see `plan.md` section 7b). `assets/audio/*.s` are generated (FamiStudio in a container, `tools/export_audio.sh`); the `.ftm` / `.txt` next to them are the sources. `tests/smoke.lua` reads the emulated APU state to check that music plays, the title is silent and everything is silent after a crash.

### Refactoring safety net

`tests/trace.lua` plays a fixed input script for 900 frames and writes one hash of the rendered frame per frame (`TRACE_OUT=file`). Run it on the ROM before and after a behaviour-preserving change and `diff` the two files; identical output means identical video. Verified for the module split: 5 runs on the new ROM, 4 identical to the pre-split ROM, 1 differed in a single frame (216) and could not be reproduced (looks like a screenshot timing race in the harness, not the ROM). Any intended art or gameplay change changes the hashes.

## Gotchas

- Font only covers: space `! " # % ( ) + - . / :`, `0-9`, `A-Z`. Anything else (lowercase, `'`, `,`, …) renders as a blank tile. Current title strings `"GEEKO MCFLY '85"` and `"WHERE WE GO, NO ROADS!"` contain `'` and `,`, so those glyphs are invisible. Add the glyph to `FONT_RAW` in `build_chr.py` to fix.
- `vram_write` lengths in `screens.c` and `hud.c` are hand-counted literals; change a string and its length together.
- Audio is FamiTone2 (draft, 2026-10-16). Never poke the APU registers (`POKE(0x40xx, ...)`) from game code: the engine rewrites every channel each frame, so a raw poke lasts one frame at most (and before the engine, a halt bit in a raw poke made the crash noise ring forever). New effects go in `assets/audio/sfx.txt` (one song per effect, in the order of the `SFX_*` ids in `sfx.c`), then `tools/export_audio.sh`.
