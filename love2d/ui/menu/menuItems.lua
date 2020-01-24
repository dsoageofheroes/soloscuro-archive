local menuItems = {}
local private = {}

local menu

local function simpleClicked(self, x, y)
  self.timer = 0
end

function menuItems.init(_items, _menu)
  menu = _menu

  local anim = Animation(2,1,4,2,1,4):setInterval(.035)

  menuItems.elements =
  {
    DsGraphic(_items.sun, 47, 25),
    DsGraphic(_items.stone, 4, 45),
    DsGraphic(_items.start, 94, 70)
      :setHover(2)
      :setClicked(simpleClicked)
      :animate(anim, function()
        menu.active = false
      end),
    DsGraphic(_items.create, 50, 87)
      :setHover(2)
      :setClicked(simpleClicked)
      :animate(anim, function()
        menu:open('CharView')
      end),
    DsGraphic(_items.load, 64, 104)
      :setHover(2)
      :setClicked(simpleClicked)
      :animate(anim),
    DsGraphic(_items.exit, 92, 120)
      :setHover(2)
      :setClicked(simpleClicked)
      :animate(anim, function()
        love.event.quit(0)
      end),
  }
end

return menuItems