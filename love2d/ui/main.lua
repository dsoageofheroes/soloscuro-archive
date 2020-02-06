require 'util'

Animation = require 'models/Animation'
Graphic = require 'models/Graphic'
Region = require 'models/Region'
Text = require 'models/Text'
dsl = {}
ds = require 'libds'

dsl.narrate_open = function (action, text, index)
    print("....")
end

-- For the original DS graphics
DsGraphic = function(assets, x, y)
  return Graphic(assets, x, y):setAspect(1.2)
end

local animation = require 'gfx/animate'
local camera = require 'gfx/camera'
local charInventory = require 'menu/inventory'
local charNav = require 'menu/nav'
local charView = require 'menu/view'
local config = require 'solconfig'
local createChar = require 'menu/create'
local draw = require 'io/draw'
local ds = require 'libds'
local font = require 'gfx/font'
local fontPatch = require 'patch/font'
local gff = require 'io/gff'
local keyboard = require 'io/keyboard'
local menu = require 'menu/menu'
local menuItems = require 'menu/menuItems'
local mouse = require 'io/mouse'
local party = require 'party'
local popup = require 'menu/popup'
local tester = require 'test/runner'

local regionTest
local devEnabled = false

function love.load(args)
  math.randomseed(os.clock())

  draw.init(config)
  camera.init(config, draw)
  local status, error = pcall(function () gff.init(ds, config) end)
  if not status then print(error.code) end
  
  mouse.init(gff.cursors, draw)
  font.init(gff.loadFontChar, fontPatch)

  popup.init(gff.popup, menu)
  menuItems.init(gff.menu, menu)
  charNav.init(gff.charOverview, menu)
  charView.init(gff.charOverview, menu, charNav)
  charInventory.init(gff.inventory, menu, charNav)
  createChar.init(gff.createChar, menu, font)
  menu.init(draw, animation, menuItems, charView, charInventory, createChar, popup)

  -- tester.init(args)

  regionTest = Region(gff, "rgn2a.gff")
end

function love.draw()

  if menu.active then
    camera.show(regionTest.map)
    menu.draw()
  else
    camera.show(regionTest.map)
  end

  draw.debug()

  if devEnabled then
    draw.absolute(gff.menu.dev, 0, 0)
  end

  -- This needs to be last so the mouse cursor is never obscured
  mouse.draw()
end

function love.update(dt)
  menu.update(dt)

  camera.update(dt)
end

function love.keypressed( key )
  if key == 'h' then 
    camera.setX(0)
    camera.setY(0) 
  end
  if key == 'm' then mouse.visible = not mouse.visible end

  if key == "d" then draw.scaleDown() end
  if key == "u" then draw.scaleUp() end

  if key == "escape" then love.event.quit(0) end

  if key == 'p' then love.graphics.captureScreenshot('cool.png') end
end

function love.mousepressed(x, y, button, istouch, presses)

  menu.clicked(x, y, button)
end
