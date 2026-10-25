-- fixme
local PROG = "/home/haves/.cache/go/bin/leds"

local isUser = false
local busy = false

local function render()
  if isUser then
    barWidget.setGlyph("lamp")
    barWidget.setGlyphColor("primary")
  else
    barWidget.setGlyph("lamp-off")
    barWidget.setGlyphColor("default")
  end
end

-- The tool prints its state line via Go's log package (stderr), so scan both.
local function applyState(result)
  local out = (result.stdout or "") .. (result.stderr or "")
  local u = out:match("user=(%a+)")
  if u then
    isUser = (u == "on")
    render()
  end
end

local function run(...)
  if busy then return end -- drop clicks while a call is in flight
  busy = true
  noctalia.runAsync({ PROG, ... }, function(result)
    busy = false
    if result.exitCode ~= 0 then
      noctalia.log(result.stderr or (PROG .. " failed"))
      noctalia.notify(PROG, "command failed, see log")
      return
    end
    applyState(result)
  end)
end

function onClick()
  run("on", isUser and "auto" or "user")
end

-- on load
render()
run()