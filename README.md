# Hack to the Future: Geeko McFly

*OpenSUSE Hack Week 26 (Nov 30 - Dec 4, 2026)*  
*"Where We're Going, We Don't Need Roads"*

A retro NES endless runner featuring **Geeko** in Marty McFly's 1985 orange puffer vest riding an anti-gravity hoverboard. Jump over bugs and tumbleweeds, duck under cyber drones, and hit **88 MPH** to trigger the time warp!

---

## Technical Specs
* **Platform**: Nintendo Entertainment System (NES)
* **Mapper**: NROM-256 (Mapper 0, 32KB PRG-ROM + 8KB CHR-ROM)
* **Compiler**: Modern C via `llvm-mos` (`mos-nes-nrom-clang`)
* **Libraries**: `neslib` + `nesdoug`
* **Video Features**: Hardware sprite-0 hit raster split (stationary HUD + smooth scrolling playfield)
* **Sound**: Direct NES APU synthesis (Pulse 1 jump, Pulse 2 chime, Triangle warp, Noise crash)

---

## Controls
| Action | Gamepad Input |
|---|---|
| **Hover Jump** | `A` Button or `D-Pad Up` |
| **Hoverboard Duck / Crouch** | `D-Pad Down` |
| **Start / Restart** | `Start` or `A` |

---

## Build Instructions

```bash
# Build ROM (auto-generates 8KB CHR-ROM and compiles)
make

# Clean build artifacts
make clean
```

Outputs: `hacktothefuture.nes` (40,976 bytes, iNES 2.0).
Playable in any NES emulator (Mesen, FCEUX, RetroArch, etc.).
