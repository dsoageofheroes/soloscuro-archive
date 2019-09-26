local view = {}
local private = {}

local menu = 1
local nav = 1

function view.init(_overview, _menu, _nav)
  menu = _menu
  nav = _nav

  view.char = {}
  addRange(view.char, nav.baseMenu())  
  addRange(view.char, nav.charView())

  view.powers = {}
  addRange(view.powers, nav.baseMenu())  
  addRange(view.powers, nav.powers())

  view.effects = {}
  addRange(view.effects, nav.baseMenu())  
  addRange(view.effects, nav.effects())

end

return view