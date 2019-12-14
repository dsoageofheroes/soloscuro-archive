local menuItems = {}
local private = {}

local menu

local function simpleClicked(self, x, y)
  self.timer = 0
end

function menuItems.init(_items, _menu)
  menu = _menu

  local anim = Animation(3, 2, 4, 3, 2, 4):setInterval(.035)

  menuItems.elements =
  {
    Graphic(_items.sun, 47, 25),
    Graphic(_items.stone, 4, 45),
    Graphic(_items.start, 94, 70)
      :setHover(2)
      :setClicked(simpleClicked)
      :animate(anim, function()
        menu.active = false
      end),
    Graphic(_items.create, 50, 87)
      :setHover(2)
      :setClicked(simpleClicked)
      :animate(anim, function()
        menu:open('CharView')
      end),
    Graphic(_items.load, 64, 104)
      :setHover(2)
      :setClicked(simpleClicked)
      :animate(anim),
    Graphic(_items.exit, 92, 120)
      :setHover(2)
      :setClicked(simpleClicked)
      :animate(anim, function()
        love.event.quit(0)
      end),
  }
end

return menuItems