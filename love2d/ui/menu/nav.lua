local nav = {}
local private = 
{
  anim = Animation(4):setInterval(.2),
  simpleClicked = function(self, x, y)
    self.timer = 0
  end,
}

local 
menu,
navItems,
baseMenu,
charViewNav,
inventoryNav,
powersNav,
effectsNav

function nav.init(_navItems, _menu)
  navItems = _navItems
  menu = _menu

  baseMenu =
  {
    DsGraphic(_navItems.sun, 47),
    DsGraphic(_navItems.panel, 0, 9),
  }

  charViewNav =
  {
    DsGraphic(_navItems.viewTitle, 56, 11),
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
    DsGraphic(_navItems.powersTitle, 109, 11),
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
    DsGraphic(_navItems.effectsTitle, 85, 11),
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
  local view = DsGraphic(navItems.viewChar, _x, _y)

  if selected then
    view.active = 4
  else
    view:setHover(2)
        :setClicked(private.simpleClicked)
        :animate(private.anim, function()
          menu:open('CharView')
        end)
  end

  return view
end

function private.inventory(_x, _y, selected)
  local inv = DsGraphic(navItems.viewInv, _x, _y)

  if selected then
    inv.active = 4
  else 
    inv:setHover(2)
       :setClicked(private.simpleClicked)
       :animate(private.anim, function()
        menu:open('Inventory')
      end)
  end

  return inv
end

function private.powers(_x, _y, selected)
  local pow = DsGraphic(navItems.viewPowers, _x, _y)

  if selected then
    pow.active = 4
  else
    pow:setHover(2)
       :setClicked(private.simpleClicked)
       :animate(private.anim, function()
        menu:open('Powers')
      end)
  end

  return pow
end

function private.effects(_x, _y, selected)
  local eff = DsGraphic(navItems.viewEffects, _x, _y)

  if selected then
    eff.active = 4
  else
    eff:setHover(2)
       :setClicked(private.simpleClicked)
       :animate(private.anim, function()
        menu:open('Effects')
      end)
  end

  return eff
end

-- Restored item; unused so far
function private.spellbook(_x, _y)
  return DsGraphic(navItems.spellbook, 138, 164):setHover(2)
end

function private.miniMenu(_x, _y)
  return DsGraphic(navItems.miniMenu, _x, _y):setActive(3)
end

function private.corona(_x, _y)
  return DsGraphic(navItems.corona, _x, _y)
          :setHover(2)
          :setClicked(function(self, button)
            menu:open('MainMenu')
          end)
end

function private.portrait(_x, _y, index)
  local portraitWidth = navItems.portraitBorder[1]:getWidth()

  return DsGraphic(navItems.portrait, _x, _y)
          :setActive(2)
          :setClicked(function(self, button)
            menu:selectedChar(index)
            local yAdjust = -4
      
            if index == 4 and menu.isOpen('Inventory') then
              yAdjust = -20
            end
      
            if button == 2 then
              menu.openPopup('createChar', _x + portraitWidth, _y + yAdjust)
            end
          end )
end

function private.portraitBorder(_x, _y, index)
  return DsGraphic(navItems.portraitBorder, _x, _y)
          :setActive(function() 
            return menu:selectedChar() == index and 4 or 1 
          end)
end

function private.leader(_x, _y, index)
  return DsGraphic(navItems.leader, _x, _y):setActive(3)
end

function private.AI(_x, _y)
  return DsGraphic(navItems.AI, _x, _y):setActive(3)
end

return nav