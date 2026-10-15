---
name: feedback-keep-docs-in-sync
description: "User wants plan.md, DIARY.md and AGENTS.md kept current as work happens, and uses \"save plan and memory\" as a command"
metadata:
  node_type: memory
  type: feedback
  originSessionId: 1b5b91a4-ef9a-4fbf-8e65-3a189a75f91f
  modified: 2026-10-12T08:30:15.331Z
---

When a decision is made or a tool/technique is used, record it in the repo docs in the same turn: `plan.md` (design, decisions, known bugs, roadmap), `DIARY.md` (dated entries + tools table with versions and gotchas), `AGENTS.md` (how to build/test, architecture). The user asked for these explicitly ("add to the plan", "add to the diary the scripted emulator development") and said "save plan and memory" at the end of a session.

**Why:** the project is a hack-week effort worked in short bursts, and the user wants history and reasons preserved (there was no git before 2026-10-12).
**How to apply:** after any change to behaviour, tooling or design, update the matching doc; keep unverified claims labelled as such (the docs do this already). "Save plan" = make sure `plan.md` is current on disk; "memory" = update this memory directory. Do not commit unless asked. See [[project-nes-game-state]].
