---
name: project-nes-game-state
description: "Where the NES \"Hack to the Future\" game stands (updated 2026-10-19), decisions the user made, what is pending"
metadata:
  node_type: memory
  type: project
  originSessionId: 1b5b91a4-ef9a-4fbf-8e65-3a189a75f91f
  modified: 2026-10-12T08:30:07.673Z
---

State on 2026-10-19 (verify with `git log` / `git status`, this goes stale):

* Public GitHub repo: https://github.com/ilmanzo/hw26-hack-to-the-future (`origin/main`). Binary images (`*.png`, `*.jpg`, `*.jpeg`) are gitignored on purpose: the concept art is linked from an external site, not stored. Licence **GPL-3.0**. `Containerfile` pins the llvm-mos SDK by SHA256.
* Design (user decisions): **RACE** (runner to 88 MPH, 60 s) **+ HACK** (single-screen rooms; hold B at a terminal). No combat, no tongue, no tail-hang (Mario-style jumping with inertia). **No death, no lives**: ONE clock for the whole game, **10 minutes** (u16 frame counter, max 1,092 s), a race crash (also -10 MPH) or foe contact costs 5 s, a hacked terminal gives 10 s; only the clock reaching 0 is a game over (screen shows score and rank, A restarts the whole game). Score: speed points, dodge streak multiplier, terminals, clean room, ranks; a flawless game tuned to about 26,000. Balance numbers are `#define`s in `src/tuning.h`, tests read them.
* **Built (2026-10-19):** the race, clock, score, rank, game-over screen, warp, **time map between levels** (Geeko blinking over the next era), **room 1 (1990) playable draft** (every era plays it), **one Geeko body for race and rooms** (`geeko.c`), blue race sky / black rooms, **MMC3 port done** (IRQ split, 73,744-byte ROM), `tests/smoke.lua` + `room.lua` + `map.lua` + `sprites.lua`, `tools/check_rooms.py` + `build_rooms.py`. Title: SELECT jumps into room 1 (dev shortcut). Backgrounds: blue race sky, black rooms, deep violet 0x03 for the boss arena (decided, not built). Not built: rooms 2-4, boss (map ends with TO BE CONTINUED), story screens, sequence table, CHR banks per era, pause.
* `plan.md` (rewritten 2026-10-19 to match the code; no references to any other platform) is the design and roadmap: section 0 status, 8 roadmap with a State column, 8a rooms, 8b boss. Open decisions in section 9 (4th epoch, lowercase font, rooms per epoch, branding of the AI agents PINCHBOT / WHIRLY / TWINKLE, time-economy amounts).
* Geeko is a **16x16 striped chameleon on a white hoverboard** (2026-10-16). Music: FamiTone2, FamiStudio as exporter (`tools/export_audio.sh`), placeholder track `chill`; the user has not listened to the effects in the game.
* `concept-art/` has 11 images (renamed by content), catalogued in `plan.md` section 2 (4 of them).
* This memory directory lives in the repo (`memory/`); `~/.claude/projects/.../memory` is a symlink to it.

**Why:** the user is building this for openSUSE Hack Week 26 (Nov 30 - Dec 4, 2026) and iterates in small steps, giving direction one message at a time.
**How to apply:** read `plan.md` first (section 0 for what exists); do not re-ask resolved decisions; do not commit unless asked. Related: [[feedback-keep-docs-in-sync]], [[reference-nes-sibling-projects]].
