-- Headless smoke test for Mesen2 (--testRunner). Run with `make test`.
-- RAM addresses are resolved by name from hacktothefuture.mlb (re-generated on every link),
-- so the test survives RAM layout changes. Mesen's working directory is not the caller's,
-- so the project root is passed in the ROOT environment variable (see `make test`).
local ROOT = os.getenv("ROOT") or "."

local RAM = emu.memType.nesInternalRam
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

local function body()
  wait(30)
  check(u8("game_state") == STATE_TITLE, "boots to title screen")

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

  check(start_game() and u16("score") == 0, "Start after game over restarts with score 0")

  -- README: "duck under cyber drones". Standing should hit a drone, ducking should not.
  plant(OBS_DRONE, 136, 10)
  xfail(u8("game_state") == STATE_GAMEOVER, "standing player is hit by a drone (README: duck under drones)")
  if u8("game_state") == STATE_GAMEOVER then start_game() end

  -- natural play: standing still must eventually crash
  poke(sym.spawn_timer, 40)
  check(wait_until(function() return u8("game_state") == STATE_GAMEOVER end, 1500),
        "idle player crashes within 1500 frames")
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
