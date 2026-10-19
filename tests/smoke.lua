-- Headless smoke test of the race for Mesen2 (--testRunner). Run with `make test`.
-- Helpers live in tests/lib.lua; balance numbers come from src/tuning.h.
local T = dofile((os.getenv("ROOT") or ".") .. "/tests/lib.lua")
local sym, u8, u16, poke, check = T.sym, T.u8, T.u16, T.poke, T.check
local wait, press, wait_until, tuning, STATE = T.wait, T.press, T.wait_until, T.tuning, T.STATE

local PENALTY = tuning("HIT_TIME_PENALTY_S") * 60      -- frames
local OBS_SIZE = 8       -- struct Obstacle in obstacles.c: x, y, width, height, type, active, hit, scored
local OBS_BUG, OBS_DRONE = 0, 2

-- Scanlines (< 240) on which the game writes PPUSCROLL mid-frame, i.e. where split() lands.
local split_lines = {}
emu.addMemoryCallback(function()
  local line = emu.getState()["ppu.scanline"]
  if line < 240 then split_lines[line] = true end
end, emu.callbackType.write, 0x2005, 0x2005)

-- Second HUD text row as the PPU shows it (nametable A row 2, columns 1..29).
local function hud_row() return T.nt_text(1, 2, 29) end

local function start_game(button)   -- Start, or another button such as "a"
  press({[button or "start"] = true}, 3)
  return wait_until(function() return u8("game_state") == STATE.PLAYING end, 10)
end

-- Put an obstacle in a slot (struct: x, y, width, height, type, active, hit, scored).
local function set_obstacle(slot, x, y, h, typ)
  poke(sym.spawn_timer, 255)  -- no random spawns while the test is in control
  for i, v in ipairs({x, y, 12, h, typ, 1, 0, 0}) do poke(sym.obstacles + slot * OBS_SIZE + i - 1, v) end
end
-- Plant obstacle 0 right on top of the player.
local function plant(typ, y, h)
  set_obstacle(0, 46, y, h, typ)
  wait(2)
end

-- Let the game spawn a drone itself (the type is random) and drop it onto the player, keeping the
-- y the game chose. Other types are discarded.
local function plant_spawned_drone()
  for _ = 1, 600 do
    poke(sym.spawn_timer, 0)
    wait(1)
    for slot = 0, 1 do
      local o = sym.obstacles + slot * OBS_SIZE
      if emu.read(o + 5, T.RAM) == 1 then
        if emu.read(o + 4, T.RAM) == OBS_DRONE then
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

local player = T.geeko("player")

local function wait_vulnerable() wait_until(function() return u8("invuln") == 0 end, 200) end

local function body()
  wait(30)
  check(u8("game_state") == STATE.TITLE, "boots to title screen")
  check(T.sounding() == "", "title screen is silent (sounding: " .. T.sounding() .. ")")
  local blank = T.blank_glyphs()
  check(blank == "", "every character on the title screen has a glyph (blank: '" .. blank .. "')")
  T.shot("title")

  check(start_game(), "Start begins the game")
  check(u8("mph") == 20, "starts at 20 MPH")
  local full = tuning("GAME_TIME_S") * 60
  check(u16("time_left") > full - 20 and u16("time_left") <= full, "the clock starts full")
  check(T.nt_text(1, 1, 25) == "TIME 10:00  YEAR 1985  X1", "HUD row 1 shows the clock, year and multiplier (found '" .. T.nt_text(1, 1, 25) .. "')")
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

  local ground = player.y()
  press({a = true}, 1)
  wait(5)
  check(player.grounded() == 0 and player.y() < ground, "A jumps")
  check(wait_until(function() return player.grounded() == 1 end, 120)
        and player.y() == ground, "lands back on the ground")

  -- The jump is the code shared with the rooms (geeko.c): a held jump rises 49 px, a tap hops less.
  local top = ground
  T.hold({a = true})
  for _ = 1, 70 do wait(1); top = math.min(top, player.y()) end
  T.hold({})
  check((ground - top) / 256 >= 47 and (ground - top) / 256 <= 51, "a held jump rises about 49 px, as in the rooms (" .. string.format("%.1f", (ground - top) / 256) .. ")")
  wait_until(function() return player.grounded() == 1 end, 120)
  wait(3)
  top = ground
  press({a = true}, 2)
  for _ = 1, 40 do wait(1); top = math.min(top, player.y()) end
  check((ground - top) / 256 > 2 and (ground - top) / 256 < 24, "letting go of A early gives a shorter jump (" .. string.format("%.1f", (ground - top) / 256) .. " px)")
  wait_until(function() return player.grounded() == 1 end, 120)
  wait(3)

  wait(40)
  check(u16("score") > 0, "score increases while playing")
  check(wait_until(function() return T.sounding():find("square1") ~= nil end, 120), "music plays while playing (Pulse 1 sounds)")
  T.shot("playing")

  -- One main-loop iteration per video frame: scroll advances exactly speed_px every frame.
  -- (A frame that overruns vblank would skip an NMI and the HUD/scroll would stutter.)
  poke(sym.spawn_timer, 255)
  local scroll0, speed = u16("scroll_x"), u8("speed_px")
  wait(30)
  check(u16("scroll_x") - scroll0 == speed * 30, "main loop keeps up with 60 fps (no dropped frames)")

  -- jumping clears a ground obstacle: no hit, no time lost beyond the frames that passed
  T.hold({a = true})                         -- hold A: a full jump
  wait_until(function() return player.y() < ground - 30 * 256 end, 30)
  local t0 = u16("time_left")
  plant(OBS_BUG, 154, 14)
  T.hold({})
  check(u8("game_state") == STATE.PLAYING and t0 - u16("time_left") <= 4, "jumping clears a ground obstacle")
  wait_until(function() return player.grounded() == 1 end, 120)

  -- A hit slows Geeko, costs time, starts the invulnerability; it does not end the game.
  poke(sym.mph, 50)
  t0 = u16("time_left")
  plant(OBS_BUG, 154, 14)
  check(u8("game_state") == STATE.PLAYING, "a crash does not end the game")
  check(u8("mph") == 50 - tuning("CRASH_MPH_LOSS"), "a crash costs " .. tuning("CRASH_MPH_LOSS") .. " MPH (now " .. u8("mph") .. ")")
  local lost = t0 - u16("time_left")
  check(lost >= PENALTY and lost <= PENALTY + 4, "a crash costs " .. tuning("HIT_TIME_PENALTY_S") .. " s of clock (lost " .. lost .. " frames)")
  check(u8("invuln") > 0, "a crash starts the invulnerability")
  t0 = u16("time_left")
  plant(OBS_BUG, 154, 14)
  check(u8("mph") == 50 - tuning("CRASH_MPH_LOSS") and t0 - u16("time_left") <= 4, "no second hit while invulnerable")
  wait_vulnerable()
  plant(OBS_BUG, 154, 14)
  check(u8("mph") <= 50 - 2 * tuning("CRASH_MPH_LOSS") + 2, "hit again once the invulnerability is over (mph " .. u8("mph") .. ")")
  T.shot("crash")

  -- Dodges: an obstacle that passes Geeko untouched gives points times the multiplier.
  wait_vulnerable()
  local DODGE = tuning("DODGE_POINTS")
  local function dodge()
    local s0, d0 = u16("score"), u16("dodges")
    set_obstacle(0, 21, 154, 14, OBS_BUG)     -- just left of Geeko, already past
    wait(2)
    return u16("score") - s0, u16("dodges") - d0
  end
  local gain, count = dodge()
  check(count == 1 and gain >= DODGE and gain <= DODGE + 8, "a dodge scores " .. DODGE .. " x1 (gained " .. gain .. ")")
  dodge(); dodge()
  check(u8("mult") == 2, "three dodges in a row raise the multiplier to 2 (mult " .. u8("mult") .. ")")
  gain = dodge()
  check(gain >= 2 * DODGE and gain <= 2 * DODGE + 8, "the next dodge scores x2 (gained " .. gain .. ")")
  plant(OBS_BUG, 154, 14)
  check(u8("mult") == 1, "a hit puts the multiplier back to 1")

  -- Drones fly at head height: a standing player is hit, a ducking one passes under.
  wait_vulnerable()
  poke(sym.mph, 60)
  t0 = u16("time_left")
  check(plant_spawned_drone(), "the game spawns a drone")
  lost = t0 - u16("time_left")
  check(u8("mph") == 60 - tuning("CRASH_MPH_LOSS") and lost >= PENALTY, "standing player is hit by a drone")
  T.shot("drone-hit")
  wait_vulnerable()
  T.hold({down = true})
  wait(3)
  poke(sym.mph, 60)
  t0 = u16("time_left")
  check(plant_spawned_drone(), "the game spawns another drone")
  check(u8("mph") == 60 and t0 - u16("time_left") <= 70, "ducking player passes under a drone")
  T.hold({})

  -- The clock running out is the only game over; the screen shows score and rank.
  poke(sym.invuln, 255)
  T.poke16("score", 14000)
  T.poke16("time_left", 120)
  check(wait_until(function() return u8("game_state") == STATE.GAMEOVER end, 130), "the game ends when the clock reaches 0")
  wait(3)   -- the text reaches the PPU through the VRAM buffer, one NMI later
  check(T.nt_text(11, 6, 9) == "GAME OVER" and T.nt_text(7, 11, 18) == "PRESS A TO RESTART",
        "game over screen says GAME OVER / PRESS A TO RESTART")
  check(T.nt_text(10, 8, 11) == string.format("SCORE %05d", u16("score")), "game over screen shows the score")
  check(T.nt_text(7, 9, 18) == "RANK SYSADMIN     ", "game over screen shows the rank (found '" .. T.nt_text(7, 9, 18) .. "')")
  local want = string.format("SPD:%02dMPH  SCR:%05d HI:%05d", u8("mph"), u16("score"), u16("hi_score"))
  check(hud_row() == want, "HUD shows the real numbers (screen '" .. hud_row() .. "', want '" .. want .. "')")
  check(T.nt_text(6, 1, 5) == "00:00", "the HUD clock shows 00:00")
  T.shot("gameover")

  check(wait_until(function() return T.sounding() == "" end, 200), "all sound stops at game over (sounding: " .. T.sounding() .. ")")
  check(start_game("a") and u16("score") == 0, "A after game over restarts with score 0")
  check(u16("time_left") > full - 20, "restart refills the clock")

  -- A clean race: with the invulnerability held up (no hits), 88 MPH takes 60 s, and the points
  -- of that race feed the model of a flawless run below.
  local frames = 0
  local reached = wait_until(function()
    poke(sym.invuln, 255)
    frames = frames + 1
    return u8("mph") >= 88
  end, 4500)
  check(reached and frames >= 3500 and frames <= 3700, "a clean race reaches 88 MPH in about 60 s (" .. frames .. " frames)")
  local race_score, race_dodges = u16("score"), u16("dodges")
  print(string.format("INFO  clean race: %d points, %d dodges, multiplier %d", race_score, race_dodges, u8("mult")))
  check(race_dodges >= 40, "a clean race passes at least 40 obstacles (" .. race_dodges .. ")")
  check(u16("time_left") >= full - frames - 60, "no time lost in the clean race")

  -- Model of a flawless game (plan.md 8a.6/8a.7): 4 clean races, 18 terminals, 5 clean rooms, and
  -- the clock left at the end times FINISH_POINTS_PER_S. Room and boss times are guesses until the
  -- rooms are measured; the target is 26,000.
  local rooms_guess, boss_guess, races = 25 + 35 + 40 + 50, 60, 4
  local spent = races * 60 + rooms_guess + boss_guess - 18 * tuning("HACK_TIME_BONUS_S")
  local left = math.min(tuning("GAME_TIME_S") - spent, tuning("TIME_MAX_S"))
  local perfect = races * race_score + 18 * tuning("TERMINAL_POINTS") + 5 * tuning("CLEAN_ROOM_POINTS")
                  + left * tuning("FINISH_POINTS_PER_S")
  print(string.format("INFO  flawless game model: %d points (races %d, terminals %d, clean rooms %d, finish %d)",
        perfect, races * race_score, 18 * tuning("TERMINAL_POINTS"), 5 * tuning("CLEAN_ROOM_POINTS"),
        left * tuning("FINISH_POINTS_PER_S")))
  check(math.abs(perfect - 26000) <= 500, "a flawless game is worth about 26,000 points (model: " .. perfect .. ")")

  -- 88 MPH starts the time warp; for its first half second the race scene is frozen, with the fire trail.
  check(u8("game_state") == STATE.WARP, "88 MPH starts the warp")

  -- Worst case for the 8-sprites-per-scanline limit: both obstacle slots at Geeko's height plus
  -- the 88 MPH fire trail (frozen in the warp's first frames).
  for slot = 0, 1 do set_obstacle(slot, 130 + 40 * slot, 154, 14, OBS_BUG) end
  local function worst(n)   -- the trail's second flame flickers, so look at several frames
    local m = 0
    for _ = 1, n do m = math.max(m, T.max_sprites_per_scanline()); wait(1) end
    return m
  end
  wait(3)
  local most = worst(8)
  check(most <= 8, "at most 8 sprites per scanline at 88 MPH (found " .. most .. ")")

  -- The warp ends in the room (1990).
  check(wait_until(function() return u8("game_state") == STATE.ROOM end, 120), "the warp ends in the room")
  wait(3)
  check(T.nt_text(1, 1, 25) == T.nt_text(1, 1, 5) .. T.nt_text(6, 1, 5) .. "  YEAR 1990  X" .. T.nt_text(25, 1, 1),
        "the HUD shows the room's year, 1990 (found '" .. T.nt_text(1, 1, 25) .. "')")
  check(T.nt_text(1, 2, 8) == "HACK 0/2", "the HUD shows the terminals to hack")
  T.shot("warp-room")
end

T.run(body, 16000)
