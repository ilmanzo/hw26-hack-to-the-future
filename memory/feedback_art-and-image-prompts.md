---
name: feedback-art-and-image-prompts
description: How the user wants Geeko art iterated and how image prompts for concept art should be written
metadata:
  node_type: memory
  type: feedback
  originSessionId: 1b5b91a4-ef9a-4fbf-8e65-3a189a75f91f
  modified: 2026-10-12T08:30:19.390Z
---

* **Geeko sprite:** the user compares against the openSUSE logo (side-view chameleon). They rejected a dog-like silhouette twice (big spiral tail, long snout) and asked for the **head to be the prominent feature** (casque spike, big ring eye, long smile), small curled tail, longer legs, no vest, no tongue. Always show a preview rendered from the real CHR/metasprite data (Pillow) before building, and iterate in small steps.
* **Concept art / box art:** the user generates images externally (the files in `concept-art/` are named `Gemini_Generated_Image_*`), so I write prompts, not images. Prompts should: say what to attach as reference images, give exact HUD/title strings (image models garble small text), avoid real brand logos (Nintendo, NES), and rename third-party mascots/products (AI agents are PINCHBOT / WHIRLY / TWINKLE instead of Clawd / ChatGPT / Gemini). Saved in `concept-art/PROMPTS.md`.

**Why:** the user is the art director and judges by eye; copyright caution was requested explicitly ("use changed name to avoid copyright").
**How to apply:** when asked for art or prompts, follow `concept-art/PROMPTS.md` conventions and append new prompts there. See [[project-nes-game-state]].
