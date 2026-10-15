# Hack to the Future: Geeko McFly

*openSUSE Hack Week 26 (Nov 30 - Dec 4, 2026)*  
*"Where We're Going, We Don't Need Roads"*

An NES game starring **Geeko**, the openSUSE chameleon, on an anti-gravity hoverboard. Jump bugs and tumbleweeds, duck under cyber drones, and race from 20 to **88 MPH**.

The finished game has two halves: the **race** (an endless runner that ends in a time warp) and the **hack** (era rooms where Geeko hacks terminals to install openSUSE, ending at a boss called The Mainframe). See [`plan.md`](plan.md).

## Status

| Piece | State |
|---|---|
| Endless runner: road, jump/duck, 3 obstacle types, score, hi-score, 20 → 88 MPH | Works, smoke-tested |
| HUD with sprite-0 scroll split, APU sound effects | Works |
| 88 MPH time warp | Only a sound for now (speed stops at 88) |
| Era rooms, terminal hacking, story screens, boss | Not started |

Known bugs are listed in [`plan.md`](plan.md) section 0.

## Controls

| Action | Input |
|---|---|
| Jump | `A` or `D-Pad Up` |
| Duck | `D-Pad Down` |
| Start / restart | `Start` or `A` |

## Build

The `Makefile` default `CC` is a path on the author's machine. Pass your own `llvm-mos` compiler, or use the container, which has the SDK pinned by version and SHA256.

```bash
# Container (podman or docker)
podman build -t llvm-mos -f Containerfile .
podman run --rm -v "$PWD":/src:Z llvm-mos make CC=mos-nes-nrom-clang

# Local llvm-mos SDK
make CC=/path/to/llvm-mos/bin/mos-nes-nrom-clang

make clean   # removes the .nes and the generated assets/graphics.chr
```

The output is `hacktothefuture.nes` (40,976 bytes, iNES 2.0). It runs in any NES emulator (Mesen, FCEUX, Nestopia, RetroArch).

`make test` runs a headless smoke test in [Mesen2](https://www.mesen.ca/) (boot, input, collisions, game over, restart, HUD split). It needs a built ROM and the Mesen path; see [`AGENTS.md`](AGENTS.md).

## Technical specs

* **Hardware target:** NES, NROM-256 (mapper 0), 32 KB PRG-ROM + 8 KB CHR-ROM.
* **Code:** C via [llvm-mos](https://github.com/llvm-mos/llvm-mos-sdk) with `neslib` and `nesdoug`.
* **Art:** all tiles are ASCII art in `tools/build_chr.py`, which generates `assets/graphics.chr`.
* **Video:** sprite-0 hit raster split (fixed HUD, scrolling playfield).
* **Sound:** raw APU register writes (Pulse 1 jump/duck, Pulse 2 beep, Triangle warp, Noise crash).

## More docs

* [`plan.md`](plan.md): game design and roadmap
* [`DIARY.md`](DIARY.md): what was done and the tool notes
* [`AGENTS.md`](AGENTS.md): architecture, runtime model and testing, for contributors and coding agents

## License

[GPL-3.0](LICENSE).

The ROM links the C runtime, `neslib` and `nesdoug` from llvm-mos-sdk. The SDK is Apache-2.0 with LLVM Exceptions; its NES libraries carry an MIT notice (© 2018 Doug Fraker). See the SDK's [LICENSE](https://github.com/llvm-mos/llvm-mos-sdk/blob/main/LICENSE).
