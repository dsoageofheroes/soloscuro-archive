local popup = {}
local private = {}

local menu = 1
local popupAssets = 1

function popup.init(_popupAssets, _menu)
  menu = _menu
  popupAssets = _popupAssets
end

function popup.createChar(_x, _y)
  local anim = { 4, interval = 0.1 }
  local simpleClicked = function(self, x, y)
    self.timer = 0
  end

  local cancel = function()
    menu.closePopup()
  end

  return
  {
    { assets = popupAssets.createChar, x = _x, y = _y },
    { assets = popupAssets.text1, x = _x + 8, y = _y + 17, animation = anim, clicked = simpleClicked, hover = 2 },
    { assets = popupAssets.text2, x = _x + 8, y = _y + 29, animation = anim, clicked = simpleClicked, hover = 2 },
    { 
      assets = popupAssets.text3, 
      x = _x + 8, 
      y = _y + 41, 
      hover = 2,
      animation = anim, 
      clicked = simpleClicked, 
      animComplete = cancel,
    },
    { 
      assets = popupAssets.corona, 
      x = _x + 102, 
      y = _y + 36, 
      hover = 2,
      clicked = cancel,
    },
  }
end

return popup