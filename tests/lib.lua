-- Shared helpers for the Mesen2 test scripts (tests/*.lua, run by `make test`). Load with
--   local T = dofile((os.getenv("ROOT") or ".") .. "/tests/lib.lua")
-- RAM addresses are resolved by name from hacktothefuture.mlb (re-generated on every link), so the
-- tests survive RAM layout changes. Mesen's working directory is not the caller's, so the project
-- root comes in the ROOT environment variable (see `make test`). Balance numbers are read from
-- src/tuning.h with T.tuning("NAME"), so the tests follow the tuned values.
local T = {}
local ROOT = os.getenv("ROOT") or "."
T.ROOT = ROOT
T.RAM = emu.memType.nesInternalRam
T.PPU = emu.memType.nesPpuMemory   -- pattern tables at $0000, nametable A at $2000
T.OAM = emu.memType.nesSpriteRam
-- game_state values, keep in step with src/main.c
T.STATE = {TITLE = 0, PLAYING = 1, GAMEOVER = 2, WARP = 3, ROOM = 4, RESULT = 5}

T.sym = {}
local mlb = io.open(ROOT .. "/hacktothefuture.mlb")
if not mlb then print("ERROR cannot open " .. ROOT .. "/hacktothefuture.mlb"); emu.stop(2); return T end
for line in mlb:lines() do
  local lo, name = line:match("^R:(%x+)[-%x]*:(%S+)$")
  if lo then T.sym[name] = tonumber(lo, 16) end
end
mlb:close()
local sym, RAM = T.sym, T.RAM
function T.u8(name)    return emu.read(sym[name], RAM) end
function T.u16(name)   return T.u8(name) + 256 * emu.read(sym[name] + 1, RAM) end
function T.poke(addr, v) emu.write(addr, v, RAM) end
function T.poke16(name, v) emu.write(sym[name], v % 256, RAM); emu.write(sym[name] + 1, v // 256, RAM) end

local tuning = {}
local th = io.open(ROOT .. "/src/tuning.h")
if th then
  for line in th:lines() do
    local name, val = line:match("^#define%s+([%w_]+)%s+(%d+)")
    if name then tuning[name] = tonumber(val) end
  end
  th:close()
end
function T.tuning(name)
  assert(tuning[name], "no numeric #define " .. name .. " in src/tuning.h")
  return tuning[name]
end

T.failures = 0
function T.check(cond, msg)
  print((cond and "PASS  " or "FAIL  ") .. msg)
  if not cond then T.failures = T.failures + 1 end
end
-- Known bug: report, don't fail the run.
function T.xfail(cond, msg)
  print((cond and "XPASS " or "XFAIL ") .. msg)
end

-- Which APU channels are really sounding right now, read from the emulated APU state (a channel
-- sounds when its length counter runs and its volume is above 0). Independent of how the sound is
-- produced, so it also catches a note that never stops.
function T.sounding()
  local st, out = emu.getState(), {}
  for _, ch in ipairs({"square1", "square2", "noise"}) do
    local p = "apu." .. ch .. ".envelope."
    local vol = st[p .. "constantVolume"] and st[p .. "volume"] or st[p .. "counter"]
    if st[p .. "lengthCounter.counter"] > 0 and vol > 0 then out[#out + 1] = ch end
  end
  if st["apu.triangle.lengthCounter.counter"] > 0 and st["apu.triangle.linearCounter"] > 0 then out[#out + 1] = "triangle" end
  return table.concat(out, ",")
end

-- Text on nametable A at (col, row), len characters.
function T.nt_text(col, row, len)
  local c = {}
  for i = 0, len - 1 do c[#c + 1] = string.char(emu.read(0x2000 + row * 32 + col + i, T.PPU)) end
  return table.concat(c)
end
function T.nt_tile(col, row) return emu.read(0x2000 + row * 32 + col, T.PPU) end

-- Characters on screen that have no glyph: text tiles (0x21..0x5F, tile index == ASCII) whose
-- pattern in the BG pattern table is blank. Returns them as a string.
function T.blank_glyphs()
  local bad = {}
  for i = 0, 959 do
    local t = emu.read(0x2000 + i, T.PPU)
    if t > 0x20 and t < 0x60 then
      local blank = true
      for b = 0, 15 do
        if emu.read(t * 16 + b, T.PPU) ~= 0 then blank = false; break end
      end
      if blank then bad[#bad + 1] = string.char(t) end
    end
  end
  return table.concat(bad)
end

-- Most sprites OAM puts on one scanline (sprite 0 included). The PPU draws only 8 per scanline
-- and drops the rest in OAM order, so more than 8 means something vanishes.
function T.max_sprites_per_scanline()
  local count, max = {}, 0
  for i = 0, 63 do
    local y = emu.read(i * 4, T.OAM)
    if y < 240 then
      for line = y + 1, y + 8 do
        count[line] = (count[line] or 0) + 1
        if count[line] > max then max = count[line] end
      end
    end
  end
  return max
end

function T.shot(name)
  local f = io.open(ROOT .. "/tests/out/" .. name .. ".png", "wb")
  if f then f:write(emu.takeScreenshot()); f:close() end
end

-- Harness: the test body is a coroutine that yields once per frame.
local held = {}
function T.hold(buttons) held = buttons end
function T.wait(n) for _ = 1, n do coroutine.yield() end end
function T.press(buttons, n) held = buttons; T.wait(n); held = {} end
function T.wait_until(fn, max)
  for _ = 1, max do
    if fn() then return true end
    coroutine.yield()
  end
  return fn()
end

-- Run body() as the test. max_frames is the watchdog (a hung run would never stop).
function T.run(body, max_frames)
  local co = coroutine.create(body)
  local done = false   -- emu.stop() is asynchronous: callbacks still fire for a few frames
  T.frame = 0          -- frames run so far (read it to time something)
  local function finish()
    done = true
    print(T.failures == 0 and "ALL PASSED" or (T.failures .. " FAILED"))
    emu.stop(T.failures == 0 and 0 or 1)
  end
  emu.addEventCallback(function() emu.setInput(held, 0) end, emu.eventType.inputPolled)
  emu.addEventCallback(function()
    if done then return end
    T.frame = T.frame + 1
    if T.frame > max_frames then T.check(false, "watchdog: test did not finish in " .. max_frames .. " frames"); return finish() end
    local ok, err = coroutine.resume(co)
    if not ok then print("ERROR " .. tostring(err)); return emu.stop(2) end
    if coroutine.status(co) == "dead" then finish() end
  end, emu.eventType.endFrame)
end

return T
