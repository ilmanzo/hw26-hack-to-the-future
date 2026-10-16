---
name: project-nes-game-state
description: "Where the NES \"Hack to the Future\" game stands (updated 2026-10-16), decisions the user made, what is pending"
metadata:
  node_type: memory
  type: project
  originSessionId: 1b5b91a4-ef9a-4fbf-8e65-3a189a75f91f
  modified: 2026-10-12T08:30:07.673Z
---

State at the end of 2026-10-12 (verify with `git log` / `git status`, this goes stale):

* Published 2026-10-15 as a **public GitHub repo**: https://github.com/ilmanzo/hw26-hack-to-the-future (`origin/main`). Binary images (`*.png`, `*.jpg`, `*.jpeg`) are gitignored on purpose: the concept art is linked from an external site, not stored, and the commit that added it was dropped from history before the first push. `concept-art/*.jpeg` and `geeko_test.png` stay on disk, untracked.
* Licence **GPL-3.0** (`LICENSE`, decided 2026-10-15). `Containerfile` pins the llvm-mos SDK by SHA256 (base image `bci-base:latest` is not pinned). README rewritten 2026-10-15.
* Design direction (user decisions, 2026-10-12): the game is **RACE** (existing runner to 88 MPH) **+ HACK**. **Hold B** to hack. **No combat**, no tongue. **No tail-hang**: Mario-style jumping on the hoverboard with inertia on start/stop. Geeko sprite is the logo-style design with a big head (user rejected "dog-like" versions).
* **Next session starts at Phase 0 of `plan.md`** (section 8): fix the known bugs (score HUD x10, drone never hits a standing player, 88 MPH warp is only a sound; README is already fixed) plus the 3 smaller ones in plan.md section 0, each with a test (`make test`).
* Pending decisions (plan.md section 9): mapper (CNROM recommended), music engine (FamiTone2 recommended, already in the SDK), 4th epoch (2020 vs 2054), rooms per epoch, branding of the AI agents (proposed names PINCHBOT / WHIRLY / TWINKLE).
* **2026-10-16 decisions:** Geeko is now a **16x16 striped chameleon on a white hoverboard** (user picked "v9"; palette 0 = light green / dark green / white, no orange; crouched pose = body squashed onto the same board; `GROUND_Y` 155). Music: **FamiStudio is the exporter** (`.ftm` -> FamiTone2 asm, run headless in a container, recipe in `plan.md` section 7b); candidate CC0 tracks are listed there (Lasso Lady, Free NES Music Pack tracks; the NES Shooter pack is VRC6-only). **Draft integrated 2026-10-16** (FamiTone2 in the game, track `chill` as placeholder, effects redone in FamiStudio text; `tools/export_audio.sh`); the user has not yet listened to it in the game. Phase 2 still has the CNROM move.
* `concept-art/` has 11 images, renamed by content on 2026-10-15 (`nes-pixel-*`, `illustration-*`, `*-box-cartridge-manual-photo`). Only 4 are catalogued in `plan.md` section 2; the other 7 are known by filename only. The low-res `17917*.jpg` copies were removed by the user.
* This memory directory lives in the repo (`memory/`); `~/.claude/projects/.../memory` is a symlink to it.

**Why:** the user is building this for openSUSE Hack Week 26 (Nov 30 - Dec 4, 2026) and iterates in small steps, giving direction one message at a time.
**How to apply:** read `plan.md` first for the full design; do not re-ask resolved decisions; do not commit unless asked. Related: [[feedback-keep-docs-in-sync]], [[reference-nes-sibling-projects]].
