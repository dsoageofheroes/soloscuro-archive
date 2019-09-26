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
    Graphic(_inventory.background),
    Graphic(_inventory.aside, 233, 4),
    Graphic(_inventory.charBackground, 75, 36),
    
    Graphic(_inventory.textbox, 53, 3):setActive(2),
    Graphic(_inventory.textbox, 46, 161):setActive(4),
    Graphic(_inventory.textbox, 57, 183):setActive(1),
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
    ammoSlot = private.slot(64, 18, 11),
    headSlot = private.slot(111, 18, 17),
    neckSlot = private.slot(159, 18, 18),
    cloakSlot = private.slot(165, 42, 22),
    chestSlot = private.slot(165, 62, 19), 
    leftHandSlot = private.slot(165, 83, 20), 
    leftFingerSlot = private.slot(165, 101, 21), 
    legSlot = private.slot(165, 121, 16), 
    footSlot = private.slot(165, 141, 23),
    waistSlot = private.slot(57, 121, 15),
    rightFingerSlot = private.slot(57, 101, 14),
    rightHandSlot = private.slot(57, 83, 13),
    armSlot = private.slot(57, 62, 10),
    bowSlot = private.slot(57, 42, 12),
  }

  inventory.elements =
  {
    menu = menuElements,
    inv = invElements
  }

end

function private.slot(_x, _y, assetIndex)
  return Graphic(invAssets.slot, _x, _y)
          :setActive(assetIndex)
          :setHover(8)
          :setClicked(private.simpleClicked)
          :animate(private.anim)
end

return inventory