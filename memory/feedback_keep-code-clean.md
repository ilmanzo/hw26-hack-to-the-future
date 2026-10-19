---
name: feedback-keep-code-clean
description: The user wants the code kept refactored: no duplication, shared helpers, clean and efficient
metadata:
  type: feedback
---

Keep the code refactored as it grows: reuse functions and similar code parts instead of copying them, and keep it clean and efficient (user, 2026-10-19). Examples done: one `struct Geeko` body for the race and the rooms (`geeko.c`), `gfx.c` for the PPU helpers room/map/cards share, shared test helpers in `tests/lib.lua`.

**Why:** the game will grow to four eras and a boss; duplicated physics or drawing code would drift apart (the race and rooms must feel the same).
**How to apply:** before adding a second copy of something, extract it; after a feature, scan the diff for duplicated blocks; compile with `-Wall -Wextra` (clean now); keep tests and docs in step. Related: [[feedback-keep-docs-in-sync]].
