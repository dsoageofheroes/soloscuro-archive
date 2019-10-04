local create = 
{
  roll = function()
    return math.ceil(math.random() * 6)
  end
}

function create.init(_createAssets, _menu, _font)
  local buttonAnim = Animation(4)
  local diceAnim = Animation(3, 4, 5, 6, 3)
  local simpleClicked = function(self)
    self.timer = 0
  end

  create.elements =
  {
    Graphic(_createAssets.background),
    Graphic(_createAssets.portraitBackground),
    Graphic(_createAssets.spriteBackground, 135, 19),
    Graphic(_createAssets.diceBackground, 129, 70),
    Graphic(_createAssets.classBackground, 208),
    Graphic(_createAssets.powersBackground, 210, 88),
    Graphic(_createAssets.buttons.exit, 258, 157)
      :setHover(2)
      :setClicked(simpleClicked)
      :animate(buttonAnim, function()
        _menu:open('CharView')
      end),
    Graphic(_createAssets.buttons.done, 243, 174)
      :setHover(2)
      :setClicked(simpleClicked)
      :animate(buttonAnim, function()
        _menu:open('CharView')
      end),
    Graphic(_createAssets.dieRoll, 139, 66)
      :setActive(create.roll() + 6)
      :setClicked(simpleClicked)
      :animate(diceAnim, function(self)
        self.active = create.roll() + 6
      end),
    Graphic(_createAssets.chars, 26, 13)
      :setClicked(function(self)
        self.active = self.active < #self.assets and self.active + 1 or 1
      end),
  }

  create.texts = 
  {
    Text("PSIONIC DISCIPLINES", _font.black, 218, 114),
    Text("TRUE NEUTRAL", _font.white, 86, 171),
    Text("MALE HUMAN", _font.white, 86, 162),
    Text([[ abcdefghijklmnopqrstuvwxyz{@|$}~^]], _font.white, 2, 180),
    Text([[ z{@}~|]], _font.white, 25, 205),
  }

end


return create