local menuItems = {}
local private = {}

local menu = 1

function menuItems.init(_items, _menu)
  menu = _menu

  local anim = { 3, 2, 4, 3, 2, 4, interval = .035 }

  function private.simpleClicked(self, x, y)
    self.timer = 0
  end

  menuItems.elements =
  {
    { assets = _items.sun, x = 47, y = 25 },
    { assets = _items.stone, x = 4, y = 45 },
    { assets = _items.start, hover = 2, x = 94, y = 70,
      animation = anim,
      clicked = private.simpleClicked,
    },
    { assets = _items.create, hover = 2, x = 50, y = 87,
      animation = anim,
      animComplete = function()
        menu:open('CharView')
      end,
      clicked = private.simpleClicked,
    },
    { assets = _items.load, hover = 2, x = 64, y = 104,
      animation = anim,
      clicked = private.simpleClicked,
    },
    { assets = _items.exit, hover = 2, x = 92, y = 120,
      animation = anim,
      animComplete = function()
        love.event.quit(0)
      end,
      clicked = private.simpleClicked,
    }
  }
end

return menuItems