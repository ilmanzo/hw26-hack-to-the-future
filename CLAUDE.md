# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

NES endless runner ("Hack to the Future: Geeko McFly", openSUSE Hack Week 26). NROM-256 (mapper 0, 32KB PRG + 8KB CHR), C via `llvm-mos` + `neslib` + `nesdoug`. No tests, no linter, no git repo.

## Build

```bash
make                # builds hacktothefuture.nes (regenerates assets/graphics.chr first if build_chr.py changed)
make clean          # removes .nes and assets/graphics.chr only (not .nes.elf / .mlb)
make CC=<path>      # override compiler
```

- `CC` is hardcoded to `/home/andrea/.local/llvm-mos/bin/mos-nes-nrom-clang`. That path does not exist on this machine; build via the container instead (below) or override `CC`.
- Link step also emits `hacktothefuture.nes.elf` and `hacktothefuture.mlb` (Mesen label file).

### Container build (no local llvm-mos)

`Containerfile` installs llvm-mos SDK (pinned `LLVM_MOS_VERSION`, default v23.3.0) on `bci-base`. Verified: output ROM is byte-identical to the checked-in `hacktothefuture.nes`.

```bash
podman build -t llvm-mos -f Containerfile .
podman run --rm -v "$PWD":/src:Z llvm-mos make CC=mos-nes-nrom-clang
```

The SDK tarball is downloaded without checksum verification.

### Headless testing (Mesen2)

Mesen2 2.1.1 is installed at `~/.local/opt/mesen2/Mesen` (Lua scripting, scriptable input, RAM reads, screenshots). Verified invocation:

```bash
LD_PRELOAD=/usr/lib64/libstdc++.so.6 ~/.local/opt/mesen2/Mesen --testRunner \
  --nes.port1.type=NesController --debug.scriptWindow.allowIoOsAccess=true \
  hacktothefuture.nes script.lua
```

- `LD_PRELOAD` is required on this system: `MesenCore.so` statically links libstdc++ and aborts with `std::bad_cast` at load without it.
- `--nes.port1.type=NesController` is required: the fresh headless config has no controller attached, so `emu.setInput` does nothing.
- `--debug.scriptWindow.allowIoOsAccess=true` is only needed for `io.open` (e.g. saving `emu.takeScreenshot()`). A Lua error in a callback hangs the runner; wrap runs in `timeout`.
- `print()` goes to stdout; `emu.stop(code)` sets the exit code. Set input inside an `emu.eventType.inputPolled` callback; do frame logic in `endFrame`.
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
   - Pattern table 0 (`$0000`, BG): tile index == ASCII code for font (0x20–0x5F), road/sky/mountain tiles at 0x80–0x88.
   - Pattern table 1 (`$1000`, sprites): Geeko ride 0x00–0x08 (24x24, 3x3 tiles, row-major) and duck 0x09–0x0E (24x16), obstacles 0x10–0x1B, FX 0x20–0x21 (fire trail, crash star). Geeko is drawn as two whole ASCII pictures (`GEEKO_RIDE`, `GEEKO_DUCK`) that `put_sprite()` slices into tiles. Edit the picture, not tiles; `sprites.h` assumes this 3-wide row-major layout.
   - Geeko-art-dependent hand-tuned numbers in `main.c`: crash burst position and the player hitbox (`pbox`: x+4, w 16). Duck art is placed at dy=7 so the hoverboard stays at the same height as when riding. Re-check these if you redraw Geeko.
2. **`assets/chr-rom.s`** `.incbin`s `graphics.chr` into the `.chr_rom` section. The Makefile finds it via `-Iassets`.
3. **`src/`** consumes those tile indices by number:
   - `sprites.h`: metasprite arrays (`dx, dy, tile, attr`, terminated by `128`) referencing sprite-table tile IDs.
   - `palettes.h`: `bg_palette` / `spr_palette`. Sprite attr palette index in `sprites.h` selects the sub-palette (0 Geeko, 1 tumbleweed, 2 bug/drone, 3 FX).
   - `main.c`: single-file game. `main()` calls `bank_spr(1)` / `bank_bg(0)` to match the CHR layout above.

### main.c runtime model

- State machine: `STATE_TITLE → STATE_PLAYING → STATE_GAMEOVER → STATE_PLAYING`, driven in one `while(1)` loop after `ppu_wait_nmi()`.
- **Sprite-0 hit split**: stationary HUD on nametable rows 0–2, scrolling playfield below. Sprite 0 (`oam_spr(240,20,0x20,3)`) must overlap tile `0x80` placed at `NTADR_A(30,2)` (`draw_hud_static`, `draw_title_screen`). Moving either, or changing tile 0x80's opaque pixels, breaks the split. `split(scroll_x)` runs only in `STATE_PLAYING`; other states force scroll 0. OAM slots 0–3 are reserved (`oam_set(4)`) for sprite 0.
- Both nametables A and B are filled by `setup_nametable()` for seamless horizontal scroll; only A gets the HUD text (HUD is above the split so B never shows it).
- HUD numbers update through the nesdoug VRAM buffer (`multi_vram_buffer_horz`), not direct PPU writes, while rendering is on. Direct `vram_*` writes require `ppu_off()`.
- Physics: 8.8 fixed point (`player_y_sub`, `GRAVITY`, `JUMP_IMPULSE`). Speed derives from `mph` (20→88, 88 triggers warp); `speed_px = 2 + mph/25`.
- OAM draw order matters: sprite 0 first, then obstacles, Geeko, fire trail. Past 8 sprites per scanline the PPU drops the lowest-priority ones; Geeko is 3 tiles wide, so keep the fire trail last.
- Collision: `nesdoug check_collision()` takes pointers to structs whose first four bytes are `x, y, width, height`. `struct Obstacle` keeps those first on purpose; do not reorder.
- Sound: raw APU register pokes (`POKE(0x40xx, …)`), no sound engine. Channel use: Pulse1 jump/duck, Pulse2 beep, Triangle warp, Noise crash.

## Gotchas

- Font only covers: space `! " # % ( ) + - . / :`, `0-9`, `A-Z`. Anything else (lowercase, `'`, `,`, …) renders as a blank tile. Current title strings `"GEEKO MCFLY '85"` and `"WHERE WE GO, NO ROADS!"` contain `'` and `,`, so those glyphs are invisible. Add the glyph to `FONT_RAW` in `build_chr.py` to fix.
- `vram_write` lengths in `main.c` are hand-counted literals; change a string and its length together.
