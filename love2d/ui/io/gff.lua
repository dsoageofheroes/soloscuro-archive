local ds = require('libds')

local gff = {
  cursors = {},
  menu = {}
}
local private = {}

-- GFF file list
local GFF_RESOURCE = "resource.gff"

function gff.init(_config)
  ds.gff_init()
  ds.gff_load_directory(_config.ds1Path)
  gff_file = ds.gff_find_index(GFF_RESOURCE)

  private.loadAllResources()
end

function private.loadAllResources()

  -- Load mouse cursor images
  for i=1,10 do
    gff.cursors[i] = private.getGraphic(gff_resource, 6000 + i, true)
  end

  gff.parchments =
  {
    smallSquare = private.getGraphic(GFF_RESOURCE, 20083),
    mediumSquare = private.getGraphic(GFF_RESOURCE, 20085),
    shortRect = private.getGraphic(GFF_RESOURCE, 20087),
    mediumRect = private.getGraphic(GFF_RESOURCE, 20086),
    tallRect = private.getGraphic(GFF_RESOURCE, 20084),
  }

  gff.buttons = 
  {
    done = private.getGraphic(GFF_RESOURCE, 2000, true),
    exit = private.getGraphic(GFF_RESOURCE, 2058, true),
    save = private.getGraphic(GFF_RESOURCE, 2057, true),
  }

  -- Load main menu assets
  gff.menu = {
    dev = love.graphics.newImage('dsun_001.png'),
    sun = private.getGraphic(GFF_RESOURCE, 20028),
    stone = private.getGraphic(GFF_RESOURCE, 20029),
    start = private.getGraphic(GFF_RESOURCE, 2048, true),
    create = private.getGraphic(GFF_RESOURCE, 2049, true),
    load = private.getGraphic(GFF_RESOURCE, 2050, true),
    exit = private.getGraphic(GFF_RESOURCE, 2051, true),
  }

  -- Load character overview
  gff.charOverview = { 
    panel = private.getGraphic(GFF_RESOURCE, 11000),
    sun = gff.menu.sun,
    viewTitle = private.getGraphic(GFF_RESOURCE, 20079),
    effectsTitle = private.getGraphic(GFF_RESOURCE, 20075),
    powersTitle = private.getGraphic(GFF_RESOURCE, 20080),
    portrait = private.getGraphic(GFF_RESOURCE, 12000),
    portraitBorder = private.getGraphic(GFF_RESOURCE, 11100, true),
    leader = private.getGraphic(GFF_RESOURCE, 11106, true),
    AI = private.getGraphic(GFF_RESOURCE, 11111, true),
    viewChar = private.getGraphic(GFF_RESOURCE, 10100, true),
    viewInv = private.getGraphic(GFF_RESOURCE, 11102, true),
    viewPowers = private.getGraphic(GFF_RESOURCE, 11103, true),
    viewEffects = private.getGraphic(GFF_RESOURCE, 11104, true),
    spellbook = private.getGraphic(GFF_RESOURCE, 11105, true), -- Restored item
    miniMenu = private.getGraphic(GFF_RESOURCE, 11101, true),
    corona = private.getGraphic(GFF_RESOURCE, 10108, true),
  }

  -- Load inventory screen
  gff.inventory = {
    background = private.getGraphic(GFF_RESOURCE, 13001),
    aside = private.getGraphic(GFF_RESOURCE, 13004),
    portrait = private.getGraphic(GFF_RESOURCE, 12000),
    portraitBorder = gff.charOverview.portraitBorder,
    leader = gff.charOverview.leader,
    AI = gff.charOverview.AI,
    slot = private.getGraphic(GFF_RESOURCE, 13007),
    chars = {},
    charBackground = private.getGraphic(GFF_RESOURCE, 13005),
    viewChar = gff.charOverview.viewChar,
    viewInv = gff.charOverview.viewInv,
    viewPowers = gff.charOverview.viewPowers,
    viewEffects = gff.charOverview.viewEffects,
    spellbook = gff.charOverview.spellbook,
    miniMenu = gff.charOverview.miniMenu,
    corona = gff.charOverview.corona,
    textbox = private.getGraphic(GFF_RESOURCE, 13000),
    statusbox = private.getGraphic(GFF_RESOURCE, 10001),
  }

  -- Load full character portraits
  for i = 0,13 do 
    gff.inventory.chars[i + 1] = private.getGraphic(GFF_RESOURCE, 20000 + i)[1]
  end

  gff.popup = 
  {
    createChar = private.getGraphic(GFF_RESOURCE, 14000),
    text1 = private.getGraphic(GFF_RESOURCE, 14101, true),
    text2 = private.getGraphic(GFF_RESOURCE, 14102, true),
    text3 = private.getGraphic(GFF_RESOURCE, 14103, true),
    corona = gff.charOverview.corona,
  }

  gff.createChar =
  {
    background = private.getGraphic(GFF_RESOURCE, 13001),
    portraitBackground = gff.parchments.tallRect,
    spriteBackground = gff.parchments.smallSquare,
    diceBackground = gff.parchments.mediumSquare,
    classBackground = gff.parchments.mediumRect,
    powersBackground = gff.parchments.shortRect,
    buttons = gff.buttons,
    dieRoll = {},
    chars = gff.inventory.chars,
  }

  for i = 0,12 do
    gff.createChar.dieRoll[i + 1] = private.getGraphic(GFF_RESOURCE, 20047 + i)[1]
  end

  -- IMAGES
  -- 3009 - save/load screen
  -- 13000 - text bar

  -- ICONS
end

function private.cropImageData(img, cropX, cropY, cropWidth, cropHeight)
  local cropped = love.image.newImageData(cropWidth, cropHeight)
  cropped:paste(img, 0, 0, cropX, cropY, cropWidth, cropHeight)
  return cropped
end

function private.getGraphic(gff_filename, res_type, isIcon)
  local graphic = {}

  local frameCount = ds.get_frame_count(gff_file, isIcon and 1313817417 or 542133570, res_type)

  for i = 1, frameCount do
    local imageData = private.getImageData(gff_file, res_type, isIcon, i - 1)
    -- graphic['data'..i] = imageData
    graphic[i] = love.graphics.newImage(imageData)
  end

  return graphic
end

function private.getImage(gff_filename, res_type, isIcon, frame, palette)
  local img = private.getImageData(gff_filename, res_type, isIcon, frame, palette)
  return love.graphics.newImage(img)
end

function private.getImageData(gff_filename, res_type, isIcon, frame, palette)
  local frame = frame or 0
  local palette = palette or 0
  local image_type_id = isIcon and 1313817417 or 542133570

  local data = ds.get_frame_rgba_with_palette(gff_file, image_type_id, res_type, frame, palette)
  local width = ds.get_frame_width(gff_file, image_type_id, res_type, frame)
  local height = ds.get_frame_height(gff_file, image_type_id, res_type, frame)
  local imageData = love.image.newImageData(width, height, "rgba8", data)
  return imageData
end

return gff
