local create = 
{
  roll = function()
    return math.ceil(math.random() * 6)
  end
}

function create.init(_createAssets, _menu, _font)
  local buttonAnim = { 4, interval = .1 }
  local diceAnim = { 3, 4, 5, 6, 3, interval = .1 }
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
    { 
      assets = _createAssets.powersBackground, 
      x = 210, 
      y = 88 
    },
    { 
      assets = _createAssets.buttons.exit, 
      x = 258, 
      y = 157, 
      hover = 2,
      clicked = simpleClicked,
      animation = buttonAnim,
      animComplete = function()
        _menu:open('CharView')
      end
    },
    { 
      assets = _createAssets.buttons.done, 
      x = 243, 
      y = 174, 
      hover = 2,
      clicked = simpleClicked,
      animation = buttonAnim,
      animComplete = function()
        _menu:open('CharView')
      end 
    },
    { 
      assets = _createAssets.dieRoll, 
      x = 139, 
      y = 66, 
      active = create.roll() + 6,
      clicked = simpleClicked,
      animation = diceAnim,
      animComplete = function(self)
        self.active = create.roll() + 6
      end
    },
    {
      assets = _createAssets.chars,
      x = 26,
      y = 13,
      active = 1,
      clicked = function(self)
        self.active = self.active < #self.assets and self.active + 1 or 1
      end
    }
  }

  create.texts = 
  {
    Text("PSI DISCIPLINES", _font.black, 226, 114),
    Text("TRUE NEUTRAL", _font.white, 86, 171),
    Text("MALE HUMAN", _font.white, 86, 162),
  }

end


return create