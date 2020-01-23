local mouse = {
  visible = true
}
local private = {}

local cursors = {}
local activeCursor = 1

local draw = 1

function mouse.init(_cursors, _draw)
  cursors = _cursors
  draw = _draw
  love.mouse.setVisible(false)
  activeCursor = cursors[1]
end

function mouse.setPointer(withNaySymbol)
  activeCursor = withNaySymbol and cursors[2] or cursors[1]
end

function mouse.setSword(withNaySymbol)
  activeCursor = withNaySymbol and cursors[3] or cursors[4]
end

function mouse.setArrow(withNaySymbol)
  activeCursor = withNaySymbol and cursors[5] or cursors[6]
end

function mouse.setEye(withNaySymbol)
  activeCursor = withNaySymbol and cursors[7] or cursors[8]
end

function mouse.setNoMagic()
  activeCursor = cursors[9]
end

function mouse.setHourglass()
  activeCursor = cursors[10]
end

function mouse.draw()
  local x = love.mouse.getX()
  local y = love.mouse.getY()

  if mouse.visible then
    draw.absolute(activeCursor[1], x, y)
  end
end

function mouse.clicked(x, y, button, istouch, presses)
  -- draw.debug2 = 'clicked: '..x..'x, '..y..'y'
end

return mouse
