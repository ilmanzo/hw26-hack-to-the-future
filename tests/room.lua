-- Headless test of room 1 (1990) for Mesen2 (--testRunner). Run with `make test`.
-- The level-select cheat on the time map (DEV_LEVEL_SELECT in src/tuning.h) starts the room.
-- Part 1 tests the pieces by teleporting Geeko (movement numbers, collision, hacking, the drone,
-- the gate, the result card); part 2 plays the whole room with a small bot that reads Geeko's and
-- the drone's position from RAM.
local T = dofile((os.getenv("ROOT") or ".") .. "/tests/lib.lua")
local sym, u8, u16, poke, check = T.sym, T.u8, T.u16, T.poke, T.check
local wait, press, wait_until, tuning, STATE = T.wait, T.press, T.wait_until, T.tuning, T.STATE

local hero = T.geeko("hero")
local function hx() return hero.x() / 256 end
local function hy() return hero.y() / 256 end
local teleport = hero.teleport
local function foe_x() return u16("foe_x") / 256 end
local function term_prog(i) return emu.read(sym.term_prog + i, T.RAM) end
local function hacked() return T.nt_text(6, 2, 1) end     -- the digit in "HACK 0/2"

local HACK_FRAMES = tuning("HACK_FRAMES_1990")
local BONUS = tuning("HACK_TIME_BONUS_S") * 60
local PENALTY = tuning("HIT_TIME_PENALTY_S") * 60

local enter_room, hold_b_until = T.enter_room, T.hold_b_until

local function body()
  if not (T.needs("DEV_INVULNERABLE", 0) and T.needs("DEV_LEVEL_SELECT", 1)) then return end
  wait(30)
  check(u8("game_state") == STATE.TITLE, "boots to title screen")
  check(enter_room(), "the level-select cheat starts room 1 from the map")
  wait(3)
  check(T.nt_text(1, 2, 8) == "HACK 0/2", "HUD row 2 shows HACK 0/2 (found '" .. T.nt_text(1, 2, 8) .. "')")
  check(T.nt_text(1, 1, 25) == "TIME 10:00  YEAR 1990  X1", "HUD row 1 shows the clock and 1990 (found '" .. T.nt_text(1, 1, 25) .. "')")
  check(T.nt_tile(0, 26) == 0xc0 and T.nt_tile(5, 27) == 0xc1, "floor tiles are drawn (solid top and body)")
  check(T.nt_tile(16, 16) == 0x90 and T.nt_tile(26, 20) == 0x90, "both terminals are drawn")
  check(T.nt_tile(0, 23) == 0x98 and T.nt_tile(1, 25) == 0x9d, "the gate is drawn, closed")
  check(hx() == 80 and hy() == 192, "Geeko starts on the floor at column 10 (x " .. hx() .. ", y " .. hy() .. ")")
  T.shot("room")

  -- Movement numbers (plan 4a): about 22 frames to full speed, about 32 to stop, jump apex about 49 px.
  local frames = 0
  T.hold({left = true})
  wait_until(function() frames = frames + 1; return hero.vx() <= -0x200 end, 60)
  check(frames >= 21 and frames <= 23, "full speed after about 22 frames (" .. frames .. ")")
  local x0 = hx()
  T.hold({})
  frames = 0
  wait_until(function() frames = frames + 1; return hero.vx() == 0 end, 60)
  local dist = x0 - hx()
  check(frames >= 31 and frames <= 34 and dist >= 27 and dist <= 35, "stops in about 32 frames and 31 px (" .. frames .. " frames, " .. string.format("%.1f", dist) .. " px)")
  T.hold({left = true})
  wait(60)
  T.hold({})
  wait(3)
  check(hx() == 0 and hero.vx() == 0, "the left screen edge is a wall (x " .. hx() .. ")")

  teleport(120, 192)    -- the drone is at 112+ on the floor: stay clear of it by jumping near the start instead
  wait(2)
  teleport(80, 192)
  wait(2)
  local top = 192
  T.hold({a = true})
  for _ = 1, 70 do wait(1); top = math.min(top, hy()) end
  T.hold({})
  check(192 - top >= 47 and 192 - top <= 51, "a full jump rises about 49 px (" .. (192 - top) .. ")")
  wait_until(function() return hero.grounded() == 1 end, 60)
  check(hy() == 192, "lands back on the floor")
  wait(3)                                   -- A must be seen released first, or there is no new press
  top = 192
  press({a = true}, 2)                      -- a tap: short hop
  for _ = 1, 50 do wait(1); top = math.min(top, hy()) end
  check(192 - top > 5 and 192 - top < 35, "letting go of A early gives a shorter jump (" .. (192 - top) .. " px)")

  -- Collision: under a platform the head stops the jump; above it Geeko lands on top.
  teleport(40, 192)                         -- platform (row 22, columns 3-9) is overhead
  wait(2)
  top = 192
  T.hold({a = true})
  for _ = 1, 40 do wait(1); top = math.min(top, hy()) end
  T.hold({})
  check(top >= 182, "the head hits the platform above (rose to y " .. top .. ")")
  wait(30)
  teleport(40, 100)
  wait(60)
  check(math.floor(hy()) == 160 and hero.grounded() == 1, "Geeko lands on the platform (y " .. hy() .. ")")

  -- The drone: a touch costs time, knocks Geeko back, starts the invulnerability.
  wait(2)
  teleport(foe_x(), 184 - 10)               -- falls onto the drone's row: overlap
  local t0 = u16("time_left")
  wait(6)
  check(u8("invuln") > 0 or u8("hit_taken") == 1, "touching the drone is a hit")
  check(t0 - u16("time_left") >= PENALTY and t0 - u16("time_left") <= PENALTY + 8, "a hit costs " .. tuning("HIT_TIME_PENALTY_S") .. " s")
  T.shot("room-hit")

  -- Hacking: hold B in the zone. Releasing resets. Finishing gives time and points.
  wait_until(function() return u8("invuln") == 0 end, 120)
  teleport(200, 160)                        -- right platform, terminal at column 26: zone x 200..231
  wait(3)
  local score0 = u16("score")
  T.hold({b = true}); wait(20); T.hold({})
  wait(2)
  check(term_prog(1) == 0 and hacked() == "0", "releasing B resets the hack")
  t0 = u16("time_left")
  frames = 0
  check(hold_b_until(function() frames = frames + 1; return hacked() == "1" end, HACK_FRAMES + 10), "holding B for " .. HACK_FRAMES .. " frames hacks the terminal")
  check(frames >= HACK_FRAMES and frames <= HACK_FRAMES + 6, "the hack takes about " .. HACK_FRAMES .. " frames (" .. frames .. ")")
  check(u16("time_left") >= t0 - frames - 2 + BONUS, "a hacked terminal gives " .. tuning("HACK_TIME_BONUS_S") .. " s")
  check(u16("score") - score0 >= tuning("TERMINAL_POINTS"), "a hacked terminal gives " .. tuning("TERMINAL_POINTS") .. " points")
  wait(3)
  check(T.nt_tile(26, 20) == 0x94 and T.nt_tile(27, 21) == 0x97, "the hacked terminal is drawn green")
  check(T.nt_tile(0, 23) == 0x98, "the gate stays shut with one terminal left")

  -- Second terminal opens the gate; reaching the gate ends the room with a result card.
  teleport(128, 128)                        -- middle platform, terminal at column 16: zone x 120..151
  wait(3)
  check(hold_b_until(function() return hacked() == "2" end, HACK_FRAMES + 10), "the second terminal is hacked")
  wait(3)
  check(T.nt_tile(0, 23) == 0x9e and T.nt_tile(1, 25) == 0xa3, "the gate opens when everything is hacked")
  teleport(4, 192)
  check(wait_until(function() return u8("game_state") == STATE.RESULT end, 10), "touching the open gate ends the room")
  wait(3)
  check(T.nt_text(8, 6, 16) == "INSTALL COMPLETE", "the result card appears")
  check(T.nt_text(8, 10, 13) == "HIT: NO BONUS", "a room with a hit gets no clean-room bonus (found '" .. T.nt_text(8, 10, 16) .. "')")
  check(T.nt_text(8, 9, 13) == "TERMINALS 2/2", "the card shows the terminals")
  T.shot("room-result")
  press({a = true}, 2)
  check(wait_until(function() return u8("game_state") == STATE.MAP end, 10), "A on the card shows the time map")
  check(u8("epoch") == 1, "the next era is 2000 (epoch " .. u8("epoch") .. ")")
  wait(tuning("MAP_SKIP_FRAMES") + 2)
  press({a = true}, 2)
  check(wait_until(function() return u8("game_state") == STATE.PLAYING end, 10), "A skips the map and starts the next race")
  wait(3)
  check(T.nt_text(1, 1, 25) == "TIME " .. T.nt_text(6, 1, 5) .. "  YEAR 1990  X1", "the next race is in 1990 (found '" .. T.nt_text(1, 1, 25) .. "')")

  -- A clean run gets the bonus: hack both without a hit.
  check(enter_room(), "enter the room again")
  wait(3)
  check(u16("score") == 0 and u16("time_left") > tuning("GAME_TIME_S") * 60 - 20, "a fresh game: score 0, full clock")
  T.finish_room()
  check(T.nt_text(8, 10, 16) == "CLEAN ROOM BONUS", "a room without a hit gets the clean-room bonus")
  check(u16("score") == 2 * tuning("TERMINAL_POINTS") + tuning("CLEAN_ROOM_POINTS"),
        "score = 2 terminals + clean room (" .. u16("score") .. ")")
  ---------------------------------------------------------------------------------------------
  -- Part 2: a bot plays the room from the start with the pad only (reads Geeko and the drone).
  ---------------------------------------------------------------------------------------------
  local function walk_to(x, tol, max)       -- run to x, braking early enough to stop on it (about 8 v^2 px)
    for _ = 1, max or 240 do
      local d = x - hx()
      local v = math.abs(hero.vx()) / 256
      if math.abs(d) <= tol and v < 0.3 then T.hold({}); return true end
      if (d > 0) == (hero.vx() >= 0) and math.abs(d) <= 8 * v * v + tol then
        T.hold({})                           -- coast
      elseif d > 0 then T.hold({right = true}) else T.hold({left = true}) end
      wait(1)
    end
    T.hold({}); return false
  end
  local function on_surface(y) return math.floor(hy()) == y and hero.grounded() == 1 end

  check(enter_room(), "bot: start the room again")
  wait(3)
  local frame0 = T.frame

  -- Route: left shelf, up to the middle shelf (terminal 1), across to the right shelf (terminal 2),
  -- back up-left to the middle shelf, off its left edge, down and left to the gate. The drone patrols
  -- the floor between columns 14 and 22 and never meets Geeko on this route.
  local function jump(dir, air, glide)      -- hold A and a direction, then glide on with the direction only
    T.hold({a = true, [dir] = true}); wait(air)
    T.hold({[dir] = true}); wait(glide)
    T.hold({}); wait(10)
  end
  check(walk_to(4, 3, 200), "bot: walks to the left of the left shelf")
  jump("right", 30, 10)
  check(on_surface(160), "bot: jumps onto the left shelf (x " .. hx() .. ", y " .. hy() .. ")")
  check(walk_to(64, 3, 200), "bot: walks to the shelf's right edge")
  jump("right", 36, 20)
  check(on_surface(128), "bot: jumps onto the middle shelf (x " .. hx() .. ", y " .. hy() .. ")")
  check(walk_to(128, 3, 200), "bot: walks to terminal 1")
  check(hold_b_until(function() return hacked() == "1" end, HACK_FRAMES + 40), "bot: hacks terminal 1")

  check(walk_to(150, 3, 200), "bot: walks towards the middle shelf's right edge")
  T.hold({right = true})                    -- walk off the edge: 32 px down onto the right shelf
  wait_until(function() return on_surface(160) end, 120)
  T.hold({}); wait(10)
  check(on_surface(160) and hx() >= 190, "bot: drops onto the right shelf (x " .. hx() .. ", y " .. hy() .. ")")
  check(walk_to(208, 3, 200), "bot: walks to terminal 2")
  check(hold_b_until(function() return hacked() == "2" end, HACK_FRAMES + 40), "bot: hacks terminal 2")
  wait(3)
  check(T.nt_tile(0, 23) == 0x9e, "bot: the gate is open")

  check(walk_to(186, 2, 200), "bot: walks to the right shelf's edge")
  jump("left", 36, 4)
  check(on_surface(128), "bot: jumps back onto the middle shelf (x " .. hx() .. ", y " .. hy() .. ")")

  T.hold({left = true})
  check(wait_until(function() return u8("game_state") == STATE.RESULT end, 600), "bot: reaches the gate")
  T.hold({})
  local elapsed = T.frame - frame0
  print(string.format("INFO  bot playthrough: %d frames (%.1f s), %s", elapsed, elapsed / 60,
        u8("hit_taken") == 1 and "hit at least once" or "no hits"))
  check(u16("time_left") > 0 and elapsed < 60 * 60, "bot: the room takes under a minute")
  check(u16("score") >= 2 * tuning("TERMINAL_POINTS"), "bot: score has the two terminals (" .. u16("score") .. ")")
  wait(3)
  T.shot("room-bot-result")
end

T.run(body, 9000)
