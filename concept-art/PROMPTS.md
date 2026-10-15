# Image prompts

Prompts for the NES package (box + cartridge + manual) of the merged game concept: RACE to 88 MPH, then HACK the terminals of each era, final boss The Mainframe (see `../plan.md`).

**Attach as reference images** (image models follow references better than words):
* `msx-box-cartridge-manual-photo.jpeg` (MSX box + cartridge + manual photo: composition, mood, Geeko with goggles, matrix rain)
* `illustration-mainframe-boss-tongue-vs-crab.jpeg` (Mainframe boss: red eye monolith, orange crab agents)
* one or two real game screenshots from `make test` (`tests/out/playing.png`, `tests/out/gameover.png`, 256x240, use as the pixel-art screenshots on the manual and box back)
* the openSUSE logo (the chameleon in the second concept image given in chat)

**Text rendering:** image models garble small text. Ask for the exact strings below, and fix the text in an editor afterwards if needed. Avoid real brand logos (Nintendo, NES, Seal of Quality, MSX): the prompts use generic stand-ins. The openSUSE logo and the AI-company-named agents need clearance before any public release (see `plan.md` section 9).

## 1. Master prompt: box + cartridge + manual flat-lay photo

```
A realistic photograph, taken from above at a slight angle, of a vintage North American 8-bit console game package from 1989 laid on a worn wooden desk: the cardboard game box on the left, the grey plastic game cartridge on the upper right, and a saddle-stitched instruction booklet leaning against the cartridge. Warm window light from the left, soft shadows, shallow depth of field, a few scratches, creased box corners, slightly yellowed paper, a little dust. Photographic realism, 4:3 landscape.

The game is "HACK TO THE FUTURE", subtitle "GEEKO McFLY". Publisher logo: a small blue square "GS" badge with the words "GEEKO SOFT". Tagline on the box: "RUN TO 88 MPH. HACK THE FUTURE." Small text: "1 PLAYER", "FOR THE 8-BIT ENTERTAINMENT SYSTEM", "HACK WEEK 26". No Nintendo or other real console logos; use a generic round gold quality-seal sticker with no text.

BOX FRONT: airbrushed 1980s painted illustration in the style of classic NES box art, framed by a thick black border with a thin neon-green stripe. Hero: Geeko, the openSUSE chameleon (teal-green, big round white eye with a black pupil, long friendly smile, a spiked crest at the back of the head, tiny aviator goggles pushed up, a spiral tail), crouching on an anti-gravity hoverboard with an orange stripe, blue plasma underneath, leaving a trail of orange fire and sparks as if hitting 88 MPH, on a neon grid road under a purple-to-orange sunset sky with distant mountains. Behind him a time-warp tunnel opens, and inside it four eras of computers stacked like a skyline: a 1990 beige tower PC with a CRT monitor, a 2000 desktop with a flat LCD, a 2010 laptop, a 2020 tablet and phone, each screen glowing green with a ">_ openSUSE" prompt. Looming at the top: a huge black monolith "mainframe" with one giant red cat-slit eye, smoking, red circuit lines, flanked by server racks, spitting small orange pixel crabs (AI agents) toward the hero. Green binary rain in the background. Title "HACK TO THE FUTURE" in a chrome-and-green slanted 1980s sci-fi logo across the top.

CARTRIDGE: grey plastic NES-style game pak with ridged top edge and grip grooves, a paper front label showing a smaller version of the box art (Geeko on the hoverboard) with the title "HACK TO THE FUTURE" and the "GEEKO SOFT" badge, metal 72-pin connector edge visible at the bottom, a small serial sticker on the back.

MANUAL: a thin booklet, cover repeating the box illustration, titled "INSTRUCTION BOOKLET". It lies slightly open showing an inner page headed "HOW TO PLAY": a drawing of a game controller with a D-pad and two red buttons, labelled "D-PAD: MOVE / GLIDE", "A: JUMP", "B: HOLD TO HACK", "START: PAUSE"; below it three tiny framed 8-bit game screenshots (a road with a hoverboard chameleon, a room with computer terminals, a boss with a red eye) and a short list of enemies with small pixel icons.

Mood: nostalgic, slightly cheeky hacker humor. Sharp focus on the box front.
```

## 2. Box front, flat illustration only (no photo)

```
Vintage 1989 NES-style game box front illustration, portrait 3:4, airbrushed painted art, thick black frame with a thin neon-green stripe. Geeko the teal-green openSUSE chameleon (big round white eye, long smile, spiked crest, aviator goggles on the forehead, spiral tail) rides an orange-striped anti-gravity hoverboard along a neon grid road, orange fire trail, sparks, speed lines, hitting 88 MPH. Behind him a time-warp tunnel shows four eras of computers glowing green with ">_ openSUSE": 1990 tower PC + CRT, 2000 desktop + flat LCD, 2010 laptop, 2020 tablet and phone. At the top a giant black monolith mainframe with one red cat-slit eye, smoke, red circuit lines, spitting orange pixel-crab robots. Green binary rain. Chrome-and-green slanted logo "HACK TO THE FUTURE", subtitle "GEEKO McFLY", tagline "RUN TO 88 MPH. HACK THE FUTURE.", bottom-left blue square "GS" badge "GEEKO SOFT", a round gold blank seal sticker. No real console logos.
```

## 3. Cartridge only

```
Studio product photo of a grey plastic NES-style game cartridge, front view, slight tilt, ridged top edge, grip grooves, metal 72-pin edge connector at the bottom. Paper label: Geeko the teal-green chameleon with aviator goggles on an orange-striped hoverboard over a neon road, title "HACK TO THE FUTURE" in a slanted chrome-and-green logo, small blue "GS / GEEKO SOFT" badge. Neutral grey background, soft light, a few scuffs. No real console or Nintendo logos.
```

## 4. Manual cover + spread only

```
Flat-lay photo of a vintage 1989 console instruction booklet: the cover (same illustration as the box: Geeko on a hoverboard at 88 MPH, time-warp tunnel of computers, red-eyed mainframe) titled "INSTRUCTION BOOKLET - HACK TO THE FUTURE", and below it an opened inner spread headed "HOW TO PLAY" with a controller diagram (D-PAD: MOVE / GLIDE, A: JUMP, B: HOLD TO HACK, START: PAUSE), a section "THE FOUR ERAS: 1990, 2000, 2010, 2020", a small enemies list with pixel icons (security drone, guard with flashlight, bug, pop-up window), and three framed 8-bit screenshots. Slightly yellowed paper, halftone print texture, staple binding. Warm light, wooden desk.
```

---

# Game phase prompts

One prompt per phase of the merged game (`../plan.md` section 1): **RACE** (run to 88 MPH), **HACK** (jump between platforms and hack the terminals), **BOSS** (The Mainframe and its evil AI agents).

**Style: pick one line and paste it at the top of each prompt.**
* **Style A, painted concept art** (like the Gemini images in this folder): `Polished 2D game concept art, side view, 16:9, bold clean outlines, rich colour, glowing lights, pixel-art HUD overlay (health/lives/score) in the corners.`
* **Style B, authentic NES screenshot:** `An authentic Nintendo Entertainment System game screenshot, 256x240 pixels shown with nearest-neighbour upscaling, hard pixel edges, no gradients, no anti-aliasing, the limited NES colour palette (a few colours per sprite, 3 colours + backdrop per background tile), 8x8 tile grid visible in the design, a pixel font in the top two text rows, at most 8 sprites on any horizontal line.`

**Geeko (use in all three).** `Geeko, the openSUSE chameleon: teal-green side-view body, one big round white eye with a black pupil, a long friendly smile, a spiked crest at the back of the head, aviator goggles pushed up, a short curled tail, standing on an anti-gravity hoverboard with an orange stripe.`

## Phase 1: RACE (run avoiding obstacles to reach critical speed)

```
[STYLE LINE]
Endless-runner stage on a neon cyber highway at almost 88 MPH. Dark night sky with a few white pixel clouds, stepped grey mountains with white peaks on the horizon, a thin bright horizon line, a road made of circuit-board tiles with a dashed centre line, solid circuit-pattern bedrock below. [GEEKO] He is mid-jump over a small red-and-purple glitch bug with antennae, while a hovering security drone with a red eye and a rolling tumbleweed of tangled wires approach from the right. Behind the hoverboard a long orange fire trail with sparks and speed lines, like a time-travel car. At the far right the air shimmers and white time-warp rings begin to open, as if the next jump will break the speed limit. HUD text at the top, exactly: "HW26: HACK TO THE FUTURE" on the first line and "SPD:87MPH  SCR:01234  HI:05000" on the second. No real brand logos.
```

## Phase 2: HACK (hack the computer, jump between platforms)

```
[STYLE LINE]
A single-screen platformer room in a 1990 UNIX lab: warm red-brick walls, stacked CRT monitors, thick cables on the floor, two tiers of steel catwalk platforms with railings. [GEEKO] He is mid-jump on the hoverboard between the lower and the upper catwalk, with a faint dotted arc showing the jump, heading for a beige tower PC with a CRT on the upper platform whose screen is blue (not hacked yet). On the lower platform another beige tower PC with a CRT already shows a green terminal with ">_ openSUSE" (hacked) and tiny cyan sparkles. A security drone patrols the floor below, casting a red cone of light, and a guard in a dark suit with a flashlight walks on the lower platform. At the far right a closed steel time-gate with a red lock light. HUD text at the top, exactly: "YEAR 1990   HACK 1/2   LIVES 3" and "SCR 000270   HI 005000". No real brand logos.
```
*Variant "holding B":* replace the jump sentence with `He stands crouched in front of the blue-screen PC, and a row of green blocks is filling a progress bar across its CRT while cyan data particles stream from the screen into it.`

## Phase 3: BOSS (The Mainframe and its evil AI agents)

```
[STYLE LINE]
Final boss arena. In the centre a huge black monolith "mainframe" with one giant red cat-slit eye and a teal swirl emblem above it, red circuit lines, smoke rising, flanked by walls of server racks and hanging cables. Three tiers of steel catwalks with railings, two computer terminals on each tier (six in total): the two terminals on the top tier have already turned green with ">_ openSUSE", the other four still glow red. [GEEKO] He is leaping between catwalks to dodge, unarmed. The mainframe spits evil AI agents at him: PINCHBOT, a blocky orange pixel-art crab robot with raised claws, angry red eyes and a cracked visor, glitching into green code; WHIRLY, a teal spiral-shaped drone; TWINKLE, a cyan four-point star-shaped sparkle bot. They fly and bounce toward him in different patterns. HUD text, exactly: boss bar at the top labelled "THE MAINFRAME" with six segments of which four are red and two are empty, then "LIVES 3" and "SCR 325000". No real brand logos.
```

## About the agent names and looks

The MSX plan names the three AI agents after real products (Clawd, ChatGPT, Gemini). For this game they are renamed and redrawn so they are not copies:

| NES name | Stand-in for | Look | Behaviour (plan.md 5) |
|---|---|---|---|
| **PINCHBOT** | Clawd | Blocky orange pixel crab: raised claws, angry red eyes, cracked visor, glitching into green code | Fast, erratic |
| **WHIRLY** | ChatGPT | Teal spiral drone | Sine-wave path |
| **TWINKLE** | Gemini | Cyan four-point star sparkle | Bouncing patrol |

Research on Clawd (Claude Code's mascot, web search 2026-10-12): an 8-bit pixel creature made of stacked orange squares (about rgb 215, 119, 87) with two black eyes, often read as a crab (its emoji is a crab). That is a recognisable trademarked character, so PINCHBOT keeps only the *family* (orange, blocky, crab-like) and changes the silhouette and face (raised claws, red eyes, visor, glitch effect) and the name. If this is ever published, have the look checked; a changed name alone is not a safe harbour.
