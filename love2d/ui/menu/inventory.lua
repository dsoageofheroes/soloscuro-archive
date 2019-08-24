local inventory = {
  enabled = false
}
local private =
{
  anim = { 4, interval = .1 },
  simpleClicked = function(self, x, y)
    self.timer = 0
  end,
}

local menuElements = {}
local invElements = {}

local invAssets = 1
local menu = 1
local nav = 1

function inventory.init(_inventory, _menu, _nav)
  invAssets = _inventory
  menu = _menu
  nav = _nav

  menuElements =
  {
    -- Main overlay
    { assets = _inventory.background, x = 0, y = 0 },
    { assets = _inventory.aside, x = 233, y = 4 },
    { assets = _inventory.charBackground, x = 75, y = 36 },
    
    { assets = _inventory.textbox, x = 53, y = 3, active = 2 },
    { assets = _inventory.textbox, x = 46, y = 161, active = 4 },
    { assets = _inventory.textbox, x = 57, y = 183, active = 1 },
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
  return
  { 
    assets = invAssets.slot, 
    animation = private.anim, 
    clicked = private.simpleClicked, 
    x = _x, 
    y = _y, 
    active = assetIndex, 
    hover = 8 
  }
end

return inventory