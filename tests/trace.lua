-- Refactor safety net: plays a fixed input script and writes one hash of the rendered frame per
-- frame to $TRACE_OUT. Run it on two ROMs (before/after a refactor) and diff the outputs:
-- identical lines = identical video for the whole run. Not part of `make test`: any intended
-- change in art or gameplay changes the hashes.
local OUT = os.getenv("TRACE_OUT") or "trace.txt"
local FRAMES = 900
local f = 0
local out = assert(io.open(OUT, "w"))

-- frame -> buttons held on that frame
local function buttons(n)
  if n >= 30 and n < 33 then return {start = true} end                  -- start the game
  for _, j in ipairs({100, 180, 260, 340, 420, 500}) do
    if n == j then return {a = true} end                                -- jumps
  end
  if n >= 200 and n < 230 then return {down = true} end                 -- duck
  if n >= 600 and n < 603 then return {start = true} end                -- restart after game over
  if n >= 640 and n < 643 then return {a = true} end                    -- jump after restart
  return {}
end

local function hash(s)
  local h = 5381
  for i = 1, #s do h = (h * 33 + s:byte(i)) % 4294967296 end
  return h
end

emu.addEventCallback(function() emu.setInput(buttons(f), 0) end, emu.eventType.inputPolled)
emu.addEventCallback(function()
  f = f + 1
  out:write(f, " ", hash(emu.takeScreenshot()), "\n")
  if f == FRAMES then out:close(); emu.stop(0) end
end, emu.eventType.endFrame)
