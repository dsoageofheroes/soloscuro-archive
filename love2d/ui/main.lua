require('util')

local popup = require('menu/popup')
local charInventory = require('menu/inventory')
local charNav = require('menu/nav')
local charView = require('menu/view')
local gff = require ('io/gff')
local mouse = require('io/mouse')
local keyboard = require('io/keyboard')
local draw = require('io/draw')
local animation = require('animation')
local menu = require('menu')
local menuItems = require('menuItems')
local party = require('party')
local music = require('music/music')
local ds = require('libds')

local devEnabled = false

function love.load()
  draw.init()
  gff.init()
  
  --Sample debug statement
  --draw.debug1 = 'Hello there!'

  mouse.init(gff.cursors, draw)
  menuItems.init(gff.menu, menu)
  charNav.init(gff.charOverview, menu)
  charView.init(gff.charOverview, menu, charNav)
  charInventory.init(gff.inventory, menu, charNav)
  menu.init(draw, animation, menuItems.elements, charView, charInventory.elements, popup(gff.popup, menu))
  music.init();
  -- Play startup music
  local midi = ds.get_chunk_as_midi(0, 1363497804, 2)
  music.music_play_mem(midi)
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
