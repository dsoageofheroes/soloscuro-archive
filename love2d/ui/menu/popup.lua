return function(_popupAssets, _menu)
  local self = {}
  local menu = _menu

  function self.createChar(_x, _y)
    local anim = { 4, interval = 0.1 }
    local simpleClicked = function(self, x, y)
      self.timer = 0
    end

    local cancel = function()
      menu.closePopup()
    end

    return
    {
      { assets = _popupAssets.createChar, x = _x, y = _y },
      { assets = _popupAssets.text1, x = _x + 8, y = _y + 17, animation = anim, clicked = simpleClicked, hover = 2 },
      { assets = _popupAssets.text2, x = _x + 8, y = _y + 29, animation = anim, clicked = simpleClicked, hover = 2 },
      { 
        assets = _popupAssets.text3, 
        x = _x + 8, 
        y = _y + 41, 
        hover = 2,
        animation = anim, 
        clicked = simpleClicked, 
        animComplete = cancel,
      },
      { 
        assets = _popupAssets.corona, 
        x = _x + 102, 
        y = _y + 36, 
        hover = 2,
        clicked = cancel,
      },
    }
  end

  return self
end