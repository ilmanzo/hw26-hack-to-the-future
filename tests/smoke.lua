-- Headless smoke test for Mesen2 (--testRunner). Run with `make test`.
-- RAM addresses are resolved by name from hacktothefuture.mlb (re-generated on every link),
-- so the test survives RAM layout changes. Mesen's working directory is not the caller's,
-- so the project root is passed in the ROOT environment variable (see `make test`).
local ROOT = os.getenv("ROOT") or "."

local RAM = emu.memType.nesInternalRam
local PPU = emu.memType.nesPpuMemory   -- pattern tables at $0000, nametable A at $2000
local OAM = emu.memType.nesSpriteRam
local STATE_TITLE, STATE_PLAYING, STATE_GAMEOVER = 0, 1, 2
local MAX_FRAMES = 4000   -- watchdog

local sym = {}
local mlb = io.open(ROOT .. "/hacktothefuture.mlb")
if not mlb then print("ERROR cannot open " .. ROOT .. "/hacktothefuture.mlb"); emu.stop(2); return end
for line in mlb:lines() do
  local lo, name = line:match("^R:(%x+)[-%x]*:(%S+)$")
  if lo then sym[name] = tonumber(lo, 16) end
end
mlb:close()
local function u8(name)  return emu.read(sym[name], RAM) end
local function u16(name) return u8(name) + 256 * emu.read(sym[name] + 1, RAM) end
local function poke(addr, v) emu.write(addr, v, RAM) end

local failures = 0
local function check(cond, msg)
  print((cond and "PASS  " or "FAIL  ") .. msg)
  if not cond then failures = failures + 1 end
end
-- Known bug: report, don't fail the run.
local function xfail(cond, msg)
  print((cond and "XPASS " or "XFAIL ") .. msg)
end

-- Scanlines (< 240) on which the game writes PPUSCROLL mid-frame, i.e. where split() lands.
local split_lines = {}
emu.addMemoryCallback(function()
  local line = emu.getState()["ppu.scanline"]
  if line < 240 then split_lines[line] = true end
end, emu.callbackType.write, 0x2005, 0x2005)

-- Which APU channels are really sounding right now, read from the emulated APU state (a channel
-- sounds when its length counter runs and its volume is above 0). Independent of how the sound is
-- produced, so it also catches a note that never stops.
local function sounding()
  local st, out = emu.getState(), {}
  for _, ch in ipairs({"square1", "square2", "noise"}) do
    local p = "apu." .. ch .. ".envelope."
    local vol = st[p .. "constantVolume"] and st[p .. "volume"] or st[p .. "counter"]
    if st[p .. "lengthCounter.counter"] > 0 and vol > 0 then out[#out + 1] = ch end
  end
  if st["apu.triangle.lengthCounter.counter"] > 0 and st["apu.triangle.linearCounter"] > 0 then out[#out + 1] = "triangle" end
  return table.concat(out, ",")
end

-- Characters on screen that have no glyph: text tiles (0x21..0x5F, tile index == ASCII) whose
-- pattern in the BG pattern table is blank. Returns them as a string.
local function blank_glyphs()
  local bad = {}
  for i = 0, 959 do
    local t = emu.read(0x2000 + i, PPU)
    if t > 0x20 and t < 0x60 then
      local blank = true
      for b = 0, 15 do
        if emu.read(t * 16 + b, PPU) ~= 0 then blank = false; break end
      end
      if blank then bad[#bad + 1] = string.char(t) end
    end
  end
  return table.concat(bad)
end

-- Second HUD text row as the PPU shows it (nametable A row 2, columns 1..29).
local function hud_row()
  local c = {}
  for col = 1, 29 do c[#c + 1] = string.char(emu.read(0x2000 + 2 * 32 + col, PPU)) end
  return table.concat(c)
end

-- Most sprites OAM puts on one scanline (sprite 0 included). The PPU draws only 8 per scanline
-- and drops the rest in OAM order, so more than 8 means something vanishes.
local function max_sprites_per_scanline()
  local count, max = {}, 0
  for i = 0, 63 do
    local y = emu.read(i * 4, OAM)
    if y < 240 then
      for line = y + 1, y + 8 do
        count[line] = (count[line] or 0) + 1
        if count[line] > max then max = count[line] end
      end
    end
  end
  return max
end

local function shot(name)
  local f = io.open(ROOT .. "/tests/out/" .. name .. ".png", "wb")
  if f then f:write(emu.takeScreenshot()); f:close() end
end

-- Harness: the test body is a coroutine that yields once per frame.
local held = {}
local function wait(n) for _ = 1, n do coroutine.yield() end end
local function press(buttons, n) held = buttons; wait(n); held = {} end
local function wait_until(fn, max)
  for _ = 1, max do
    if fn() then return true end
    coroutine.yield()
  end
  return fn()
end

local function start_game()
  press({start = true}, 3)
  return wait_until(function() return u8("game_state") == STATE_PLAYING end, 10)
end

-- Plant obstacle 0 right on top of the player (struct: x, y, width, height, type, active).
local function plant(typ, y, h)
  poke(sym.spawn_timer, 255)  -- no random spawns while the test is in control
  for i, v in ipairs({46, y, 12, h, typ, 1}) do poke(sym.obstacles + i - 1, v) end
  wait(2)
end
local OBS_BUG, OBS_DRONE = 0, 2

-- Let the game spawn a drone itself (the type is random) and drop it onto the player, keeping the
-- y the game chose. Other types are discarded.
local function plant_spawned_drone()
  for _ = 1, 600 do
    poke(sym.spawn_timer, 0)
    wait(1)
    for slot = 0, 1 do
      local o = sym.obstacles + slot * 6
      if emu.read(o + 5, RAM) == 1 then
        if emu.read(o + 4, RAM) == OBS_DRONE then
          poke(sym.spawn_timer, 255)
          poke(o, 46)
          wait(2)
          return true
        end
        poke(o + 5, 0)
      end
    end
  end
  return false
end

local function body()
  wait(30)
  check(u8("game_state") == STATE_TITLE, "boots to title screen")
  check(sounding() == "", "title screen is silent (sounding: " .. sounding() .. ")")
  local blank = blank_glyphs()
  check(blank == "", "every character on the title screen has a glyph (blank: '" .. blank .. "')")
  shot("title")

  check(start_game(), "Start begins the game")
  check(u8("mph") == 20, "starts at 20 MPH")
  poke(sym.spawn_timer, 255)

  -- The HUD occupies scanlines 8..23 (tile rows 1-2); the scroll split must land on 23 so the
  -- playfield scrolls from 24. A hit earlier scrolls the bottom strokes of the HUD text.
  split_lines = {}
  wait(20)
  local lines = {}
  for line in pairs(split_lines) do lines[#lines + 1] = line end
  check(#lines == 1 and lines[1] == 23, "scroll split lands on scanline 23 (found: " .. table.concat(lines, ",") .. ")")

  press({down = true}, 5)
  check(u8("player_ducking") == 1, "Down ducks")
  wait(2)
  check(u8("player_ducking") == 0, "releasing Down stands up")

  local ground = u16("player_y_sub")
  press({a = true}, 1)
  wait(5)
  check(u8("player_jumping") == 1 and u16("player_y_sub") < ground, "A jumps")
  check(wait_until(function() return u8("player_jumping") == 0 end, 120)
        and u16("player_y_sub") == ground, "lands back on the ground")

  wait(40)
  check(u16("score") > 0, "score increases while playing")
  check(wait_until(function() return sounding():find("square1") ~= nil end, 120), "music plays while playing (Pulse 1 sounds)")
  shot("playing")

  -- One main-loop iteration per video frame: scroll advances exactly speed_px every frame.
  -- (A frame that overruns vblank would skip an NMI and the HUD/scroll would stutter.)
  poke(sym.spawn_timer, 255)
  local scroll0, speed = u16("scroll_x"), u8("speed_px")
  wait(30)
  check(u16("scroll_x") - scroll0 == speed * 30, "main loop keeps up with 60 fps (no dropped frames)")

  -- jump clears a ground obstacle
  press({a = true}, 1)
  wait_until(function() return u16("player_y_sub") < ground - 30 * 256 end, 30)
  plant(OBS_BUG, 154, 14)
  check(u8("game_state") == STATE_PLAYING, "jumping clears a ground obstacle")
  wait_until(function() return u8("player_jumping") == 0 end, 120)

  -- standing player hits a ground obstacle
  plant(OBS_BUG, 154, 14)
  check(u8("game_state") == STATE_GAMEOVER, "standing player crashes into a ground obstacle")
  shot("gameover")
  wait(3)   -- the HUD numbers reach the PPU through the VRAM buffer, one NMI later
  local want = string.format("SPD:%02dMPH  SCR:%05d HI:%05d", u8("mph"), u16("score"), u16("hi_score"))
  check(hud_row() == want, "HUD shows the real numbers (screen '" .. hud_row() .. "', want '" .. want .. "')")

  -- Regression: the crash noise used to be started with the length-counter halt bit set and never
  -- stopped, also after a restart. Music stops on the crash; the crash effect lasts under 60 frames.
  check(wait_until(function() return sounding() == "" end, 200), "all sound stops after the crash (sounding: " .. sounding() .. ")")
  wait(120)
  check(sounding() == "", "still silent 2 s later (sounding: " .. sounding() .. ")")

  check(start_game() and u16("score") == 0, "Start after game over restarts with score 0")

  -- Drones fly at head height: a standing player is hit, a ducking one passes under.
  check(plant_spawned_drone(), "the game spawns a drone")
  check(u8("game_state") == STATE_GAMEOVER, "standing player is hit by a drone")
  wait(3)
  shot("drone-hit")
  check(start_game(), "restart after the drone crash")
  held = {down = true}
  wait(3)
  check(plant_spawned_drone(), "the game spawns another drone")
  check(u8("game_state") == STATE_PLAYING, "ducking player passes under a drone")
  held = {}

  -- natural play: standing still must eventually crash
  poke(sym.spawn_timer, 40)
  check(wait_until(function() return u8("game_state") == STATE_GAMEOVER end, 1500),
        "idle player crashes within 1500 frames")

  -- Worst case for the 8-sprites-per-scanline limit: both obstacle slots at Geeko's height plus
  -- the 88 MPH fire trail.
  check(start_game(), "restart for the sprite-limit check")
  poke(sym.spawn_timer, 255)
  poke(sym.score, 2720 % 256); poke(sym.score + 1, 2720 // 256)   -- mph is derived from score: 20 + score/40
  check(wait_until(function() return u8("mph") == 88 end, 10), "reaches 88 MPH")
  for slot = 0, 1 do
    for i, v in ipairs({100 + 40 * slot, 154, 12, 14, OBS_BUG, 1}) do poke(sym.obstacles + slot * 6 + i - 1, v) end
  end
  wait(3)
  check(u8("game_state") == STATE_PLAYING, "worst-case scene is still playing")
  local function worst(frames)   -- the trail's second flame flickers, so look at several frames
    local m = 0
    for _ = 1, frames do m = math.max(m, max_sprites_per_scanline()); wait(1) end
    return m
  end
  local most = worst(8)
  check(most <= 8, "at most 8 sprites per scanline while playing at 88 MPH (found " .. most .. ")")
  plant(OBS_BUG, 154, 14)
  most = worst(8)
  print("INFO  crashed at 88 MPH: " .. most .. " sprites on the busiest scanline")
end

local co = coroutine.create(body)
local frame = 0
local done = false   -- emu.stop() is asynchronous: callbacks still fire for a few frames
local function finish()
  done = true
  print(failures == 0 and "ALL PASSED" or (failures .. " FAILED"))
  emu.stop(failures == 0 and 0 or 1)
end

emu.addEventCallback(function() emu.setInput(held, 0) end, emu.eventType.inputPolled)
emu.addEventCallback(function()
  if done then return end
  frame = frame + 1
  if frame > MAX_FRAMES then check(false, "watchdog: test did not finish in " .. MAX_FRAMES .. " frames"); return finish() end
  local ok, err = coroutine.resume(co)
  if not ok then print("ERROR " .. tostring(err)); return emu.stop(2) end
  if coroutine.status(co) == "dead" then finish() end
end, emu.eventType.endFrame)
