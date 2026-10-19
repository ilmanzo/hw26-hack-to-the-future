-- Headless test of the time map for Mesen2 (--testRunner). Run with `make test`.
-- The map is shown before each race: the eras on a line, finished ones green, Geeko blinking over the
-- next one; A or Start skips it after MAP_SKIP_FRAMES, otherwise it ends by itself after MAP_FRAMES.
local T = dofile((os.getenv("ROOT") or ".") .. "/tests/lib.lua")
local u8, check, wait, press, wait_until, tuning, STATE = T.u8, T.check, T.wait, T.press, T.wait_until, T.tuning, T.STATE
local NODE_COL = {4, 10, 16, 22, 28}
local NODE_ROW = 10

-- Frames (out of n) in which a sprite stands above node i (Geeko's x is the node's column x 8).
local function geeko_frames_over(i, n)
  local seen = 0
  for _ = 1, n do
    for s = 1, 63 do
      if emu.read(s * 4 + 3, T.OAM) == NODE_COL[i] * 8 and emu.read(s * 4, T.OAM) < 240 then seen = seen + 1; break end
    end
    wait(1)
  end
  return seen
end

local function nodes() -- the first tile of every node: 0xa8 waiting, 0xac done, 0xb0 the boss
  local t = {}
  for i = 1, 5 do t[i] = string.format("%02x", T.nt_tile(NODE_COL[i], NODE_ROW)) end
  return table.concat(t, " ")
end

local function body()
  if not T.needs("DEV_LEVEL_SELECT", 1) then return end
  wait(30)
  press({start = true}, 3)
  check(wait_until(function() return u8("game_state") == STATE.MAP end, 10), "Start on the title screen shows the time map")
  wait(3)
  check(T.nt_text(12, 5, 8) == "TIME MAP" and T.nt_text(11, 17, 9) == "NEXT STOP", "the map has its title and caption")
  check(T.nt_text(3, 13, 4) == "1990" and T.nt_text(27, 13, 4) == "2054", "the eras are labelled with their years")
  check(T.nt_text(11, 19, 9) == "UNIX LABS", "the next stop is named (found '" .. T.nt_text(11, 19, 9) .. "')")
  check(nodes() == "a8 a8 a8 a8 b0", "no era done yet, the boss at the end (" .. nodes() .. ")")
  check(T.palette(0) == 0x0f, "the map has a black background")
  check(T.nt_text(1, 1, 25) == "TIME 10:00  YEAR 1985  X1", "the HUD stays on the map (found '" .. T.nt_text(1, 1, 25) .. "')")
  T.shot("map")

  local on = geeko_frames_over(1, 32)
  check(on >= 12 and on <= 20, "Geeko blinks over the first era (visible in " .. on .. " of 32 frames)")
  check(u8("game_state") == STATE.MAP, "still on the map")
  press({a = true}, 2)
  check(wait_until(function() return u8("game_state") == STATE.PLAYING end, 10), "A skips the map after the minimum time")
  wait(5)

  -- A pressed too early is ignored: the map is still there after 10 frames.
  T.poke(T.sym.game_state, STATE.TITLE)
  press({start = true}, 3)
  wait_until(function() return u8("game_state") == STATE.MAP end, 10)
  press({a = true}, 2)
  check(u8("game_state") == STATE.MAP, "A in the first frames does not skip the map")
  -- and without any input the map ends by itself after MAP_FRAMES
  check(wait_until(function() return u8("game_state") == STATE.PLAYING end, tuning("MAP_FRAMES") + 5), "the map ends by itself and the race starts")

  -- After a room the next era is up: the first one is green, Geeko blinks over the second.
  check(T.enter_room(), "enter room 1")
  T.finish_room()
  press({a = true}, 2)
  check(wait_until(function() return u8("game_state") == STATE.MAP end, 10), "the result card leads to the map")
  wait(3)
  check(nodes() == "ac a8 a8 a8 b0", "the first era is done (" .. nodes() .. ")")
  check(T.nt_text(8, 19, 15) == " DOT-COM FARM  ", "the next stop is the 2000 era")
  check(geeko_frames_over(2, 32) >= 12, "Geeko blinks over the second era")
  T.shot("map-2")

  -- The boss is not built: its map ends the game with TO BE CONTINUED, with score and rank.
  check(T.enter_room(), "enter room 1 again")
  T.poke(T.sym.epoch, 3)
  T.finish_room()
  press({a = true}, 2)
  check(wait_until(function() return u8("game_state") == STATE.MAP end, 10), "the last room leads to the map")
  wait(3)
  check(nodes() == "ac ac ac a8 b0" or nodes() == "ac ac ac ac b0", "three eras done (" .. nodes() .. ")")
  check(T.nt_text(9, 19, 13) == "THE MAINFRAME" and geeko_frames_over(5, 32) >= 12, "Geeko blinks over the boss")
  check(wait_until(function() return u8("game_state") == STATE.GAMEOVER end, tuning("MAP_FRAMES") + 5), "the boss map ends the game")
  wait(3)
  check(T.nt_text(8, 6, 15) == "TO BE CONTINUED", "the ending says TO BE CONTINUED")
  check(T.nt_text(7, 9, 5) == "RANK ", "the ending shows the rank")
  T.shot("ending")
  press({a = true}, 2)
  check(wait_until(function() return u8("game_state") == STATE.MAP end, 10) and u8("epoch") == 0, "A starts a new game at the first era")

  -- The level-select cheat (DEV_LEVEL_SELECT): LEFT/RIGHT pick a level, A races it, SELECT enters its room.
  check(T.open_map(), "open the map for the cheat")
  check(T.nt_text(4, 26, 24) == "LEFT/RIGHT: PICK A LEVEL", "the map shows the cheat's keys")
  press({right = true}, 2); wait(3)
  check(u8("epoch") == 1 and nodes() == "ac a8 a8 a8 b0", "RIGHT picks the second era (" .. nodes() .. ")")
  check(T.nt_text(8, 19, 15) == " DOT-COM FARM  ", "the caption follows (found '" .. T.nt_text(8, 19, 15) .. "')")
  check(geeko_frames_over(2, 32) >= 12, "Geeko blinks over the picked era")
  for _ = 1, 4 do press({right = true}, 2); wait(3) end
  check(u8("epoch") == 4 and nodes() == "ac ac ac ac b0", "RIGHT stops at the boss (epoch " .. u8("epoch") .. ")")
  press({left = true}, 2); wait(3)
  press({left = true}, 2); wait(3)
  check(u8("epoch") == 2 and nodes() == "ac ac a8 a8 b0", "LEFT goes back (" .. nodes() .. ")")
  check(wait_until(function() return u8("game_state") ~= STATE.MAP end, tuning("MAP_FRAMES") + 20) == false, "a picked level does not time out")
  T.shot("map-pick")
  press({a = true}, 2)
  check(wait_until(function() return u8("game_state") == STATE.PLAYING end, 10), "A races the picked era")
  wait(3)
  check(T.nt_text(1, 1, 25) == "TIME " .. T.nt_text(6, 1, 5) .. "  YEAR 2000  X1", "the race is in the era's year (found '" .. T.nt_text(1, 1, 25) .. "')")
  check(T.enter_room(3), "SELECT enters the room of the fourth era")
  wait(3)
  check(u8("epoch") == 3 and T.nt_text(1, 1, 25) == "TIME 10:00  YEAR 2020  X1", "room of 2020 (found '" .. T.nt_text(1, 1, 25) .. "')")
  check(T.open_map(), "open the map again")
  for _ = 1, 4 do press({right = true}, 2); wait(3) end
  press({select = true}, 2)
  check(wait_until(function() return u8("game_state") == STATE.GAMEOVER end, 10), "SELECT on the boss ends the game")
  wait(3)
  check(T.nt_text(8, 6, 15) == "TO BE CONTINUED", "the boss is not built: TO BE CONTINUED")
end

T.run(body, 4000)
