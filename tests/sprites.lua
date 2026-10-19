-- Sprites per scanline for Mesen2 (--testRunner). Run with `make test`.
-- The PPU draws at most 8 sprites per scanline and drops the rest in OAM order, so every scene the
-- game can show is measured here (OAM read through the sprite RAM), in its worst arrangement.
local T = dofile((os.getenv("ROOT") or ".") .. "/tests/lib.lua")
local u8, poke, check, wait, wait_until, STATE = T.u8, T.poke, T.check, T.wait, T.wait_until, T.STATE
local OBS_BUG, OBS_DRONE = T.OBS_BUG, T.OBS_DRONE

local function busiest(frames)       -- the fire trail's second flame flickers: look at several frames
  local m = 0
  for _ = 1, frames do m = math.max(m, T.max_sprites_per_scanline()); wait(1) end
  return m
end
local function report(scene, found, bound)   -- bound: what the scene can add up to, itself at most 8
  print(string.format("INFO  %-58s %d sprites on the busiest scanline", scene, found))
  check(found <= bound and bound <= 8, scene .. ": at most " .. bound .. " per scanline (found " .. found .. ")")
end

local function body()
  if not (T.needs("DEV_INVULNERABLE", 0) and T.needs("DEV_LEVEL_SELECT", 1)) then return end
  wait(30)
  check(T.start_game(), "start a game")
  poke(T.sym.mph, 80)                                  -- fast, but under 88: no fire trail in the race

  -- The race: two ground obstacles at Geeko's height (Geeko 2 + 2 x 2).
  T.set_obstacle(0, 130, 154, 14, OBS_BUG); T.set_obstacle(1, 170, 154, 14, OBS_BUG)
  wait(2)
  report("race: Geeko and two ground obstacles", busiest(4), 6)

  -- A drone at head height over a ground obstacle: more rows overlap, the count stays the same.
  T.set_obstacle(0, 130, 147, 10, OBS_DRONE); T.set_obstacle(1, 170, 154, 14, OBS_BUG)
  wait(2)
  report("race: a drone and a ground obstacle", busiest(4), 6)

  -- The crash pose and burst (Geeko ducking + the star) with two obstacles in the same rows: the first
  -- 20 frames after a hit.
  T.plant(OBS_BUG, 154, 14)
  T.set_obstacle(1, 100, 154, 14, OBS_BUG)
  local m = 0
  for _ = 1, 8 do m = math.max(m, T.max_sprites_per_scanline()); wait(1) end
  report("race: the crash burst, Geeko and two obstacles", m, 7)
  wait_until(function() return u8("invuln") == 0 end, 100)

  -- 88 MPH: the warp starts, the scene freezes with the fire trail (2 flames) and both obstacles.
  poke(T.sym.mph, 87)
  check(wait_until(function() return u8("game_state") == STATE.WARP end, 80), "88 MPH starts the warp")
  T.set_obstacle(0, 130, 154, 14, OBS_BUG); T.set_obstacle(1, 170, 154, 14, OBS_BUG)
  wait(3)
  report("warp: Geeko, two obstacles and the fire trail", busiest(8), 8)

  -- A room: Geeko, a drone, a knock back.
  check(wait_until(function() return u8("game_state") == STATE.ROOM end, 120), "the warp ends in the room")
  wait(3)
  report("room: Geeko and the drone", busiest(4), 4)
  T.geeko("hero").teleport(T.u16("foe_x") / 256, 184 - 10)
  wait(6)
  report("room: Geeko knocked back by the drone", busiest(4), 4)

  -- The map: Geeko only.
  T.finish_room()
  T.press({a = true}, 2)
  check(wait_until(function() return u8("game_state") == STATE.MAP end, 10), "the map")
  local seen = 0
  for _ = 1, 16 do seen = math.max(seen, T.max_sprites_per_scanline()); wait(1) end
  report("map: Geeko blinking over an era", seen, 2)
end

T.run(body, 2000)
