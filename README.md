# Hack to the Future: Geeko McFly

*openSUSE Hack Week 26 (Nov 30 - Dec 4, 2026)*  
*"Where We're Going, We Don't Need Roads"*

An NES game starring **Geeko**, the openSUSE chameleon, on an anti-gravity hoverboard. Jump bugs and tumbleweeds, duck under cyber drones, and race from 20 to **88 MPH**.

The finished game has two halves: the **race** (an endless runner that ends in a time warp) and the **hack** (era rooms where Geeko hacks terminals to install openSUSE, ending at a boss called The Mainframe). See [`plan.md`](plan.md).

## Status

| Piece | State |
|---|---|
| Race: road, jump/duck, 3 obstacle types, 20 → 88 MPH in about 60 s; a crash slows you and costs time, never ends the run | Works, smoke-tested |
| One 10-minute clock for the whole game; game over only when it runs out, with score and rank | Works |
| Score: speed, dodge streaks (x1 to x5), hacked terminals, clean rooms; ranks NEWBIE to ROOT | Works |
| HUD with an MMC3 IRQ scroll split, FamiTone2 sound effects and a draft music track | Works |
| 88 MPH time warp: flash, the year rolls, the room appears | Works (draft) |
| Room 1 (1990): run and jump, hack two terminals (hold `B`), a patrolling drone, the gate | Playable draft, tested by a bot |
| Time map between levels (eras on a line, Geeko blinking over the next) | Works |
| MMC3 port (scanline IRQ split) | Done |
| Rooms for the other eras, story screens, the boss | Not started |

Known bugs are listed in [`plan.md`](plan.md) section 0.

## Controls

| Action | Input |
|---|---|
| Jump | `A` or `D-Pad Up` |
| Duck (race) | `D-Pad Down` |
| Run (room) | `D-Pad Left` / `Right` |
| Hack a terminal (room) | hold `B` next to it |
| Start / restart, skip the map | `Start` or `A` |
| Jump straight into room 1 (development shortcut on the title screen) | `Select` |

## Build

The `Makefile` default `CC` is a path on the author's machine. Pass your own `llvm-mos` compiler, or use the container (`make podman`), which has the SDK pinned by version and SHA256.

```bash
# Container (podman or docker)
podman build -t llvm-mos -f Containerfile .
podman run --rm -v "$PWD":/src:Z llvm-mos make CC=mos-nes-mmc3-clang

# Local llvm-mos SDK
make CC=/path/to/llvm-mos/bin/mos-nes-mmc3-clang

make clean   # removes the .nes and the generated assets/graphics.chr
```

The output is `hacktothefuture.nes` (73,744 bytes, iNES 2.0). It runs in any NES emulator (Mesen, FCEUX, Nestopia, RetroArch).

`make test` runs headless tests in [Mesen2](https://www.mesen.ca/): the race, clock, score and game over, and a bot that plays room 1. It needs a built ROM and the Mesen path; see [`AGENTS.md`](AGENTS.md).

## Technical specs

* **Hardware target:** NES, MMC3 (mapper 4): 64 KB PRG-ROM image + 8 KB CHR-ROM so far (more CHR banks for the eras are planned).
* **Code:** C via [llvm-mos](https://github.com/llvm-mos/llvm-mos-sdk) with `neslib` and `nesdoug`.
* **Art:** all tiles are ASCII art in `tools/build_chr.py`, which generates `assets/graphics.chr`.
* **Video:** MMC3 scanline IRQ split (fixed HUD, scrolling playfield); blue sky in the race, black rooms.
* **Sound:** FamiTone2 music and effects (music exported from FamiTracker with FamiStudio, `tools/export_audio.sh`).

## More docs

* [`plan.md`](plan.md): game design and roadmap
* [`DIARY.md`](DIARY.md): what was done and the tool notes
* [`AGENTS.md`](AGENTS.md): architecture, runtime model and testing, for contributors and coding agents

## Credits

* Music (draft): `nes_23-chill` by Phillip Miller ("redactionary"), from the [Free NES Music Pack](https://opengameart.org/content/free-nes-music-pack) on OpenGameArt, CC0 (the author's `read_me.txt`: free for anything, with or without attribution). Source: `assets/audio/chill.ftm`.

## License

[GPL-3.0](LICENSE).

The ROM links the C runtime, `neslib` and `nesdoug` from llvm-mos-sdk. The SDK is Apache-2.0 with LLVM Exceptions; its NES libraries carry an MIT notice (© 2018 Doug Fraker). See the SDK's [LICENSE](https://github.com/llvm-mos/llvm-mos-sdk/blob/main/LICENSE).
