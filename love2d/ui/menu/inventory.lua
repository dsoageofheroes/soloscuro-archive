local inventory = {
  enabled = false
}
local private =
{
  anim = Animation(4),
  simpleClicked = function(self, x, y)
    self.timer = 0
  end,
}

local 
menuElements,
invElements,
invAssets,
menu,
nav

function inventory.init(_inventory, _menu, _nav)
  invAssets = _inventory
  menu = _menu
  nav = _nav

  menuElements =
  {
    -- Main overlay
    DsGraphic(_inventory.background),
    DsGraphic(_inventory.aside, 233, 4),
    DsGraphic(_inventory.charBackground, 75, 36),
    
    DsGraphic(_inventory.textbox, 53, 3):setActive(2),
    DsGraphic(_inventory.textbox, 46, 161):setActive(4),
    DsGraphic(_inventory.textbox, 57, 183):setActive(1),
  }

  addRange(menuElements, nav.inventory())

  -- Slot hovers (these still need to be implemented)
  -- 5 - yellow border, used to indicate eligible target slots for an item
  -- 7 - red X out, used to indicate that a selected slot cannot store the intended item

  invElements =
  {
    -- Unequipped slots
    private.slot(186, 18, 3),
    private.slot(204, 18, 3),
    private.slot(186, 36, 3),
    private.slot(204, 36, 3),
    private.slot(186, 54, 3),
    private.slot(204, 54, 3),
    private.slot(186, 72, 3),
    private.slot(204, 72, 3),
    private.slot(186, 90, 3),
    private.slot(204, 90, 3),
    private.slot(186, 108, 3),
    private.slot(204, 108, 3),

    -- Equipped Slots
    private.slot(64, 18, 11),   -- Ammo
    private.slot(111, 18, 17),  -- Head
    private.slot(159, 18, 18),  -- Neck
    private.slot(165, 42, 22),  -- Cloak
    private.slot(165, 62, 19),  -- Chest
    private.slot(165, 83, 20),  -- Left Hand
    private.slot(165, 101, 21), -- Left Finger
    private.slot(165, 121, 16), -- Legs
    private.slot(165, 141, 23), -- Feet
    private.slot(57, 121, 15),  -- Waist
    private.slot(57, 101, 14),  -- Right Finger
    private.slot(57, 83, 13),   -- Right Hand
    private.slot(57, 62, 10),   -- Arms
    private.slot(57, 42, 12),   -- Bow
  }

  inventory.elements =
  {
    menu = menuElements,
    inv = invElements
  }

end

function private.slot(_x, _y, assetIndex)
  return DsGraphic(invAssets.slot, _x, _y)
          :setActive(assetIndex)
          :setHover(8)
          :setClicked(private.simpleClicked)
          :animate(private.anim)
end

return inventory