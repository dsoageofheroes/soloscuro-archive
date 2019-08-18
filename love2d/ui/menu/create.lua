local create = {}

function create.init(_createAssets, _menu)
  local buttonAnim = { 4, interval = .1 }
  local simpleClicked = function(self)
    self.timer = 0
  end

  create.elements =
  {
    { assets = _createAssets.background, x = 0, y = 0 },
    { assets = _createAssets.portraitBackground, x = 0, y = 0 },
    { assets = _createAssets.spriteBackground, x = 135, y = 19 },
    { assets = _createAssets.diceBackground, x = 129, y = 70 },
    { assets = _createAssets.classBackground, x = 208, y = 0 },
    { assets = _createAssets.powersBackground, x = 210, y = 88 },
    { 
      assets = _createAssets.buttons.exit, 
      x = 258, 
      y = 154, 
      hover = 2,
      clicked = simpleClicked,
      animation = buttonAnim,
      animComplete = function()
        _menu:open('CharView')
      end
    },
    { assets = _createAssets.buttons.done, x = 243, y = 174, hover = 2 },
  }

end


return create