--[[ PUBLIC MEMBERS

.init(_items, _draw, _charOverview)
.draw()
.clicked(x, y)

]]

local menuItems = 1
local draw = 1
local animation = 1
local view = 1
local inventory = 1
local createChar = 1
local popup = 1

local private = 
{
  -- Forms
  isMainMenuOpen = true,
  isCharViewOpen = false,
  isInventoryOpen = false,
  isPowersOpen = false,
  isEffectsOpen = false,
  isCreateCharOpen = false,
  isActivePopupOpen = false,

  -- Popups
  activePopupWindow = false,
}

local menu = 
{
  selectedCharIndex = 1,
  selectedChar = function(self, index)
    if index then self.selectedCharIndex = index end
    return self.selectedCharIndex
  end,
  open = function(self, formId)
    self.closeAll()
    private['is'..formId..'Open'] = true
  end,
  closeAll = function()
    private.isMainMenuOpen = false
    private.isCharViewOpen = false
    private.isInventoryOpen = false
    private.isPowersOpen = false
    private.isEffectsOpen = false
    private.isCreateCharOpen = false
    private.isActivePopupOpen = false
  end,
  close = function(name)
    private['is'..name..'Open'] = false
  end,
  isOpen = function(name)
    return private['is'..name..'Open']
  end,
  openPopup = function(elementName, _x, _y)
    private.activePopupWindow = popup[elementName](_x, _y)
    private.isActivePopupOpen = true
  end,
  closePopup = function()
    private.isActivePopupOpen = false
  end,
}

function menu.init(_draw, _animation, _menuItems, _view, _inventory, _createChar, _popup)
  menuItems = _menuItems
  draw = _draw
  animation = _animation
  view = _view
  inventory = _inventory
  createChar = _createChar
  popup = _popup
end

function menu.draw()
  
  if private.isMainMenuOpen then
    draw.collection(menuItems.elements)
  end

  if private.isCharViewOpen then
    draw.collection(view.char)
  end

  if private.isInventoryOpen then
    draw.collection(inventory.menuElements)
    draw.collection(inventory.invElements)
  end

  if private.isPowersOpen then
    draw.collection(view.powers)
  end

  if private.isEffectsOpen then
    draw.collection(view.effects)
  end

  if private.isCreateCharOpen then
    draw.collection(createChar.elements)
  end

  if private.isActivePopupOpen then
    draw.collection(private.activePopupWindow)
  end

end

function menu.update(dt)

  if private.isActivePopupOpen then
    animation.updateCollection(dt, private.activePopupWindow)
  end

  if private.isMainMenuOpen then
    animation.updateCollection(dt, menuItems.elements)
  end

  if private.isCharViewOpen then
    animation.updateCollection(dt, view.char)
  end

  if private.isInventoryOpen then
    animation.updateCollection(dt, inventory.menuElements)
    animation.updateCollection(dt, inventory.invElements)
  end

  if private.isPowersOpen then
    animation.updateCollection(dt, view.powers)
  end

  if private.isEffectsOpen then
    animation.updateCollection(dt, view.effects)
  end

  if private.isCreateCharOpen then
    animation.updateCollection(dt, createChar.elements)
  end

end

function menu.clicked(x, y, button)

  if private.isActivePopupOpen then
    draw.clickItemIn(private.activePopupWindow, x, y, button)
    return
  end

  if private.isMainMenuOpen then
    draw.clickItemIn(menuItems.elements, x, y, button)
  end

  if private.isCharViewOpen then
    draw.clickItemIn(view.char, x, y, button)
  end

  if private.isInventoryOpen then
    draw.clickItemIn(inventory.menuElements, x, y, button)
    draw.clickItemIn(inventory.invElements, x, y, button)
  end

  if private.isPowersOpen then
    draw.clickItemIn(view.powers, x, y, button)
  end

  if private.isEffectsOpen then
    draw.clickItemIn(view.effects, x, y, button)
  end

  if private.isCreateCharOpen then
    draw.clickItemIn(createChar.elements, x, y, button)
  end

end

return menu