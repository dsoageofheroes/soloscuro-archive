local nav = {}
local private = 
{
  anim = { 4, interval = .2 },
  simpleClicked = function(self, x, y)
    self.timer = 0
  end,
}

local menu = 1
local navItems = 1

local baseMenu = 1
local charViewNav = 1
local inventoryNav = 1
local powersNav = 1
local effectsNav = 1

function nav.init(_navItems, _menu)
  navItems = _navItems
  menu = _menu

  baseMenu =
  {
    { assets = _navItems.sun, x = 47, y = 0 },
    { assets = _navItems.panel, x = 0, y = 9 },
  }

  charViewNav =
  {
    { assets = _navItems.viewTitle, x = 56, y = 11 },
    private.charView(43, 164, true),
    private.inventory(67, 164),
    private.powers(91, 164),
    private.effects(114, 164),
    private.miniMenu(223, 164),
    private.corona(253, 164),
    private.portrait(55, 40, 1),
    private.portrait(106, 40, 2),
    private.portrait(55, 100, 3),
    private.portrait(106, 100, 4),
    private.portraitBorder(53, 39, 1),
    private.portraitBorder(104, 39, 2),
    private.portraitBorder(53, 99, 3),
    private.portraitBorder(104, 99, 4),
    private.leader(43, 39, 1),
    private.leader(94, 39, 2),
    private.leader(43, 99, 3),
    private.leader(94, 99, 4),
    private.AI(43, 48, 1),
    private.AI(94, 48, 2),
    private.AI(43, 108, 3),
    private.AI(94, 108, 4),
  }

  inventoryNav =
  {
    private.charView(163, 181),
    private.inventory(187, 181, true),
    private.powers(211, 181),
    private.effects(235, 181),
    private.miniMenu(258, 181),
    private.corona(288, 181),
    private.portrait(14, 6, 1),
    private.portrait(14, 54, 2),
    private.portrait(14, 102, 3),
    private.portrait(14, 150, 4),
    private.portraitBorder(12, 5, 1),
    private.portraitBorder(12, 53, 2),
    private.portraitBorder(12, 101, 3),
    private.portraitBorder(12, 149, 4),
    private.leader(2, 5, 1),
    private.leader(2, 53, 2),
    private.leader(2, 101, 3),
    private.leader(2, 149, 4),
    private.AI(2, 14, 1),
    private.AI(2, 62, 2),
    private.AI(2, 110, 3),
    private.AI(2, 158, 4),
  }

  powersNav =
  {
    { assets = _navItems.powersTitle, x = 109, y = 11 },
    private.charView(43, 164),
    private.inventory(67, 164),
    private.powers(91, 164, true),
    private.effects(114, 164),
    private.miniMenu(223, 164),
    private.corona(253, 164),
    private.portrait(55, 40, 1),
    private.portrait(106, 40, 2),
    private.portrait(55, 100, 3),
    private.portrait(106, 100, 4),
    private.portraitBorder(53, 39, 1),
    private.portraitBorder(104, 39, 2),
    private.portraitBorder(53, 99, 3),
    private.portraitBorder(104, 99, 4),
    private.leader(43, 39, 1),
    private.leader(94, 39, 2),
    private.leader(43, 99, 3),
    private.leader(94, 99, 4),
    private.AI(43, 48, 1),
    private.AI(94, 48, 2),
    private.AI(43, 108, 3),
    private.AI(94, 108, 4),
  }

  effectsNav =
  {
    { assets = _navItems.effectsTitle, x = 85, y = 11 },
    private.charView(43, 164),
    private.inventory(67, 164),
    private.powers(91, 164),
    private.effects(114, 164, true),
    private.miniMenu(223, 164),
    private.corona(253, 164),
    private.portrait(55, 40, 1),
    private.portrait(106, 40, 2),
    private.portrait(55, 100, 3),
    private.portrait(106, 100, 4),
    private.portraitBorder(53, 39, 1),
    private.portraitBorder(104, 39, 2),
    private.portraitBorder(53, 99, 3),
    private.portraitBorder(104, 99, 4),
    private.leader(43, 39, 1),
    private.leader(94, 39, 2),
    private.leader(43, 99, 3),
    private.leader(94, 99, 4),
    private.AI(43, 48, 1),
    private.AI(94, 48, 2),
    private.AI(43, 108, 3),
    private.AI(94, 108, 4),
  }

end

function nav.baseMenu()
  return baseMenu
end

function nav.charView()
  return charViewNav
end

function nav.inventory()
  return inventoryNav
end

function nav.powers()
  return powersNav
end

function nav.effects()
  return effectsNav
end

function private.charView(_x, _y, selected)
  local view = 
  { 
    assets = navItems.viewChar, 
    x = _x, 
    y = _y, 
    hover = 2, 
    animation = private.anim, 
    animComplete = 
    function()
      menu:open('CharView')
    end,
    clicked = private.simpleClicked
  }

  if selected then
    view.clicked = nil
    view.hover = nil
    view.active = 4
  end

  return view
end

function private.inventory(_x, _y, selected)
  local inv =
  { 
    assets = navItems.viewInv, 
    x = _x, 
    y = _y, 
    hover = 2, 
    animation = private.anim,
    animComplete = function()
      menu:open('Inventory')
    end,
    clicked = private.simpleClicked
  }

  if selected then
    inv.clicked = nil
    inv.hover = nil
    inv.active = 4
  end

  return inv
end

function private.powers(_x, _y, selected)
  local pow = 
  { 
    assets = navItems.viewPowers, 
    x = _x, 
    y = _y, 
    hover = 2,
    animation = private.anim,
    animComplete = function()
      menu:open('Powers')
    end,
    clicked = private.simpleClicked
  }

  if selected then
    pow.clicked = nil
    pow.hover = nil
    pow.active = 4
  end

  return pow
end

function private.effects(_x, _y, selected)
  local eff = 
  { 
    assets = navItems.viewEffects, 
    x = _x, 
    y = _y, 
    hover = 2,
    animation = private.anim,
    animComplete = function()
      menu:open('Effects')
    end,
    clicked = private.simpleClicked
  }

  if selected then
    eff.clicked = nil
    eff.hover = nil
    eff.active = 4
  end

  return eff
end

-- Restored item; unused so far
function private.spellbook(_x, _y)
  return { assets = navItems.spellbook, x = 138, y = 164, hover = 2 }
end

function private.miniMenu(_x, _y)
  return { assets = navItems.miniMenu, x = _x, y = _y, active = 3 }
end

function private.corona(_x, _y)
  return
  { 
    assets = navItems.corona, 
    x = _x, 
    y = _y, 
    hover = 2,
    clicked = function(self, button)
      menu:open('MainMenu')
    end
  }
end

function private.portrait(_x, _y, index)
  local portraitWidth = navItems.portraitBorder[1]:getWidth()

  return
  { 
    assets = navItems.portrait, 
    x = _x, 
    y = _y, 
    active = 2,
    clicked = function(self, button)
      menu:selectedChar(index)

      if button == 2 then
        menu.openPopup('createChar', _x + portraitWidth, _y - 4)
      end
    end 
  }
end

function private.portraitBorder(_x, _y, index)
  return
  { 
    assets = navItems.portraitBorder, 
    x = _x, 
    y = _y, 
    active = function() 
      return menu:selectedChar() == index and 4 or 1 
    end 
  }
end

function private.leader(_x, _y, index)
  return { assets = navItems.leader, x = _x, y = _y, active = 3 }
end

function private.AI(_x, _y)
  return { assets = navItems.AI, x = _x, y = _y, active = 3 }
end

return nav