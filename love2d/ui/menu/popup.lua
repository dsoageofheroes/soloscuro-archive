local popup = {}
local private = {}

local menu = 1
local popupAssets = 1

function popup.init(_popupAssets, _menu)
  menu = _menu
  popupAssets = _popupAssets
end

function popup.createChar(_x, _y)
  local anim = Animation(4):setInterval(.1)
  local simpleClicked = function(self, x, y)
    self.timer = 0
  end

  local cancel = function()
    menu.closePopup()
  end

  local elements = 
  {
    DsGraphic(popupAssets.createChar, _x, _y),
    DsGraphic(popupAssets.text1, _x + 8, _y + 17)
      :setHover(2)
      :setClicked(simpleClicked)
      :animate(anim, function()
        menu:open('CreateChar')
      end),
    DsGraphic(popupAssets.text2, _x + 8, _y + 29)
      :setHover(2)
      :setClicked(simpleClicked)
      :animate(anim),
    DsGraphic(popupAssets.text3, _x + 8, _y + 41)
      :setHover(2)
      :setClicked(simpleClicked)
      :animate(anim, cancel),
    DsGraphic(popupAssets.corona, _x + 102, _y + 36)
      :setHover(2)
      :setClicked(cancel),
  }

  return 
  {
    elements = elements
  }
end

return popup