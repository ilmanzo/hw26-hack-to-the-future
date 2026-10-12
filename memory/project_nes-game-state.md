---
name: project-nes-game-state
description: "Where the NES \"Hack to the Future\" game stands (2026-10-12), decisions the user made, what is pending"
metadata:
  node_type: memory
  type: project
  originSessionId: 1b5b91a4-ef9a-4fbf-8e65-3a189a75f91f
  modified: 2026-10-12T08:30:07.673Z
---

State at the end of 2026-10-12 (verify with `git log` / `git status`, this goes stale):

* Everything done in the first session is **committed** (code modules + tests + tools, docs, concept art; see `git log`). Untracked on purpose: `geeko_test.png` (unknown origin, left alone).
* Design direction (user decisions, 2026-10-12): the game is **RACE** (existing runner to 88 MPH) **+ HACK**. **Hold B** to hack. **No combat**, no tongue. **No tail-hang**: Mario-style jumping on the hoverboard with inertia on start/stop. Geeko sprite is the logo-style design with a big head (user rejected "dog-like" versions).
* **Next session starts at Phase 0 of `plan.md`** (section 8): fix the 4 known bugs (score HUD x10, drone never hits a standing player, 88 MPH warp is only a sound, README out of date) plus the 3 smaller ones in plan.md section 0, each with a test (`make test`).
* Pending decisions (plan.md section 9): mapper (CNROM recommended), music engine (FamiTone2 recommended, already in the SDK), 4th epoch (2020 vs 2054), rooms per epoch, licence of this repo, branding of the AI agents (proposed names PINCHBOT / WHIRLY / TWINKLE).
* `concept-art/` has 7 newer Gemini images (probably renders from `PROMPTS.md`) that nobody has looked at or catalogued yet; the low-res `17917*.jpg` copies were removed by the user.
* This memory directory lives in the repo (`memory/`); `~/.claude/projects/.../memory` is a symlink to it.

**Why:** the user is building this for openSUSE Hack Week 26 (Nov 30 - Dec 4, 2026) and iterates in small steps, giving direction one message at a time.
**How to apply:** read `plan.md` first for the full design; do not re-ask resolved decisions; do not commit unless asked. Related: [[feedback-keep-docs-in-sync]], [[reference-nes-sibling-projects]].
