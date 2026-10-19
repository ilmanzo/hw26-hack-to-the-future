-- The development cheats of src/tuning.h do what their switches say, for Mesen2 (--testRunner).
-- DEV_LEVEL_SELECT and DEV_INVULNERABLE are build options: this test follows whichever way the ROM
-- was built, so it also proves that a release build (both 0) has no cheat in it.
local T = dofile((os.getenv("ROOT") or ".") .. "/tests/lib.lua")
local u8, u16, poke, check, wait, press, wait_until, tuning, STATE = T.u8, T.u16, T.poke, T.check, T.wait, T.press, T.wait_until, T.tuning, T.STATE

local function blinking(frames)      -- frames (out of n) in which Geeko's sprites are drawn (OAM slot 0 is Geeko in the race)
  local drawn = 0
  for _ = 1, frames do
    if emu.read(0, T.OAM) < 240 then drawn = drawn + 1 end
    wait(1)
  end
  return drawn
end

local function body()
  wait(30)

  -- Level select: LEFT/RIGHT and SELECT on the map.
  check(T.open_map(), "open the map")
  press({right = true}, 2); wait(3)
  if tuning("DEV_LEVEL_SELECT") == 1 then
    check(u8("epoch") == 1, "level select ON: RIGHT picks the next level")
    check(T.nt_text(4, 26, 24) == "LEFT/RIGHT: PICK A LEVEL", "level select ON: the map shows the keys")
  else
    check(u8("epoch") == 0, "level select OFF: RIGHT does nothing")
    check(T.nt_text(4, 26, 24) ~= "LEFT/RIGHT: PICK A LEVEL", "level select OFF: no hint on the map")
    press({select = true}, 2); wait(3)
    check(u8("game_state") == STATE.MAP, "level select OFF: SELECT does not start a room")
  end

  -- Invulnerability: nothing hits Geeko, who blinks all the time.
  T.poke(T.sym.game_state, STATE.TITLE)
  check(T.start_game(), "start a game")
  poke(T.sym.mph, 50)
  local t0 = u16("time_left")
  T.plant(T.OBS_BUG, 154, 14)
  if tuning("DEV_INVULNERABLE") == 1 then
    check(u8("mph") == 50 and t0 - u16("time_left") <= 4, "invulnerability ON: an obstacle on Geeko does nothing")
    local zero = false
    for _ = 1, 150 do if u8("invuln") == 0 then zero = true end; wait(1) end
    check(not zero, "invulnerability ON: it never runs out")
    local drawn = blinking(40)
    check(drawn >= 12 and drawn <= 28, "invulnerability ON: Geeko blinks (drawn in " .. drawn .. " of 40 frames)")
    if tuning("DEV_LEVEL_SELECT") == 1 then
      check(T.enter_room(), "enter room 1")
      T.geeko("hero").teleport(T.u16("foe_x") / 256, 184 - 10)
      t0 = u16("time_left")
      wait(10)
      check(t0 - u16("time_left") <= 12, "invulnerability ON: the drone costs no time")
    end
  else
    check(u8("mph") == 40, "invulnerability OFF: a hit costs 10 MPH (mph " .. u8("mph") .. ")")
    wait(tuning("INVULN_FRAMES") + 5)
    check(u8("invuln") == 0, "invulnerability OFF: it runs out")
  end
end

T.run(body, 3000)
