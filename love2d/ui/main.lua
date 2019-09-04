require 'util'

local config = require 'solconfig'
local popup = require 'menu/popup'
local charInventory = require 'menu/inventory'
local charNav = require 'menu/nav'
local charView = require 'menu/view'
local createChar = require 'menu/create'
local gff = require 'io/gff'
local mouse = require 'io/mouse'
local keyboard = require 'io/keyboard'
local draw = require 'io/draw'
local animation = require 'animation'
local menu = require 'menu'
local menuItems = require 'menuItems'
local party = require 'party'

local devEnabled = false

math.randomseed(os.clock())

function love.load()
  draw.init()
  gff.init(config)
  mouse.init(gff.cursors, draw)

  popup.init(gff.popup, menu)
  menuItems.init(gff.menu, menu)
  charNav.init(gff.charOverview, menu)
  charView.init(gff.charOverview, menu, charNav)
  charInventory.init(gff.inventory, menu, charNav)
  createChar.init(gff.createChar, menu)
  menu.init(draw, animation, menuItems, charView, charInventory, createChar, popup)
end

function love.draw()

  menu.draw()
  
  draw.debug()

  if devEnabled then
    draw.absolute(gff.menu.dev, 0, 0)
  end

  -- This needs to be last so the mouse cursor is never obscured
  mouse.draw()
end

function love.update(dt)
  menu.update(dt)
end

function love.keypressed( key )
  if key == 'h' then devEnabled = not devEnabled end
  if key == 'm' then mouse.visible = not mouse.visible end

  if key == "down" then draw.scaleDown() end
  if key == "up" then draw.scaleUp() end
end

function love.mousepressed(x, y, button, istouch, presses)

  menu.clicked(x, y, button)
end
