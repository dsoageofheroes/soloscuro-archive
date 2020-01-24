local create = 
{
  roll = function()
    return  math.random(6)
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
    DsGraphic(_createAssets.background),
    DsGraphic(_createAssets.portraitBackground),
    DsGraphic(_createAssets.spriteBackground, 135, 19),
    DsGraphic(_createAssets.diceBackground, 129, 70),
    DsGraphic(_createAssets.classBackground, 208),
    DsGraphic(_createAssets.powersBackground, 210, 88),
    DsGraphic(_createAssets.buttons.exit, 258, 157)
      :setHover(2)
      :setClicked(simpleClicked)
      :animate(buttonAnim, function()
        _menu:open('CharView')
      end),
    DsGraphic(_createAssets.buttons.done, 243, 174)
      :setHover(2)
      :setClicked(simpleClicked)
      :animate(buttonAnim, function()
        _menu:open('CharView')
      end),
    DsGraphic(_createAssets.dieRoll, 139, 66)
      :setActive(create.roll() + 6)
      :setClicked(simpleClicked)
      :animate(diceAnim, function(self)
        self.active = create.roll() + 6
      end),
    DsGraphic(_createAssets.chars, 26, 13)
      :setClicked(function(self)
        self.active = self.active < #self.assets and self.active + 1 or 1
      end),
  }

  create.texts = 
  {
    Text("NAME: Mor'bad Tha Wurst", _font.white, 5, 150),
    Text("STR:17", _font.white, 11, 162),
    Text("DEX:16", _font.white, 11, 170),
    Text("CON:15", _font.white, 10, 178),
    Text("INT:18", _font.white, 11, 186),
    Text("WIS:17", _font.white, 12, 194),
    Text("CHA:12", _font.white, 12, 202),
    Text("PSIONIC DISCIPLINES", _font.black, 218, 114),
    Text("MALE HUMAN", _font.white, 53, 162),
    Text("LAWFUL NEUTRAL", _font.white, 53, 170),
    Text("FIGHTER", _font.white, 53, 178),
    Text("PSIONICIST", _font.white, 53, 186),
    Text("PRESERVER", _font.white, 53, 194),
    Text("LEVEL: 2/2/2", _font.white, 135, 162),
    Text("AC: 10", _font.white, 135, 170),
    Text("HP: 36/36", _font.white, 135, 178),
    Text("PSP: 90/90", _font.white, 135, 186),
    Text("EXP: 4000 (8000)", _font.white, 135, 194),
  }

end


return create