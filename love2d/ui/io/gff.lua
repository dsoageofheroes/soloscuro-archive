local gff = {
  cursors = {},
  menu = {}
}
local private = {}

local PALETTE = 0

-- These the are hardcoded indexes of important GFFs
local RESOURCE_GFF_INDEX = 0
local OBJEX_GFF_INDEX = 1
local DSLDATA_GFF_INDEX = 2
local CHARSAVE_GFF_INDEX = 3
local DARKSAVE_GFF_INDEX = 4
local CINE_GFF_INDEX = 5

local ds

-- GFF file list
local GFF_RESOURCE = "resource.gff"

-- GFF type list
local RMAP_TYPE = 1346456914
local GMAP_TYPE = 1346456903
local TILE_TYPE = 1162627412
local ETAB_TYPE = 1111577669
local RDFF_TYPE = 1179010130
local BMP_TYPE  = 542133570
local MONR_TYPE = 1380863821
local FONT_TYPE = 1414418246

function gff.init(_ds, _config)
  ds = _ds
  ds.gff_init()
  ds.gff_load_directory(_config.env.path.ds1)

  private.loadMenuResources()
end

function gff.loadFontChar(i)
  local char
  local img, w, h = ds.create_font_img(0, i, 0xFFFFFFFF, 0x000000FF);
  
  if w > 0 then
    char = love.image.newImageData(w, h, "rgba8", img)
  end

  return char
end

function gff.loadTiles(file) 
  local file_id = ds.gff_find_index(file)
  local tile_id_list = ds.gff_get_id_list(file_id, TILE_TYPE);
  local tiles = {}

  local paletteId = ds.gff_get_palette_id(DSLDATA_GFF_INDEX, 41)
  for k,tile_id in pairs(tile_id_list) do
      local data = ds.get_frame_rgba_with_palette(file_id, TILE_TYPE, tile_id, 0, paletteId)
      if (data ~=  0) then -- always check that it got the image!
          local width = ds.get_frame_width(file_id, TILE_TYPE, tile_id, 0)
          local height = ds.get_frame_height(file_id, TILE_TYPE, tile_id, 0)
          local imageData = love.image.newImageData(width, height, "rgba8", data)
          tiles[tile_id] = love.graphics.newImage( imageData )
      end
  end

  return tiles
end

function gff.loadMap(file, tiles)
  local file_id = ds.gff_find_index(file)
  local res_id = ds.gff_get_id_list(file_id, RMAP_TYPE)[1]
  local rmapstartx = 0
  local rmapstarty = 0
  local rmapx = 0
  local rmapy = 0
  local map = {}

  local rmap = ds.gff_get_data_as_text(file_id, RMAP_TYPE, res_id)
  for i=rmapstartx,126 do
      for j=rmapstarty,96 do -- This may need to be 97!
          local nextx = rmapx + 16*(i-rmapstartx) -- tiles are 16x16
          local nexty = rmapy + 16*(j-rmapstarty) -- tiles are 16x16
          local tile_id = string.byte(rmap, i+j*128+1)
          table.insert(map, DsGraphic(tiles[tile_id], nextx, nexty))
      end
  end

  return map
end

function private.loadMenuResources()

  -- Load mouse cursor images
  for i=1,10 do
    gff.cursors[i] = private.getGraphic(GFF_RESOURCE, 6000 + i, true)
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
  local fileIndex = ds.gff_find_index(gff_filename)

  local frameCount = ds.get_frame_count(fileIndex, isIcon and 1313817417 or 542133570, res_type)
  local paletteId = ds.gff_get_palette_id(RESOURCE_GFF_INDEX, 0)

  for i = 1, frameCount do
    local imageData = private.getImageData(fileIndex, res_type, isIcon, i - 1, paletteId)
    -- graphic['data'..i] = imageData
    graphic[i] = love.graphics.newImage(imageData)
  end

  return graphic
end

function private.getImage(gff_filename, res_type, isIcon, frame, palette)
  local img = private.getImageData(gff_filename, res_type, isIcon, frame, palette)
  return love.graphics.newImage(img)
end

function private.getImageData(fileIndex, res_type, isIcon, frame, palette)
  local frame = frame or 0
  local palette = palette or 0
  local image_type_id = isIcon and 1313817417 or 542133570

  local data = ds.get_frame_rgba_with_palette(fileIndex, image_type_id, res_type, frame, palette)
  local width = ds.get_frame_width(fileIndex, image_type_id, res_type, frame)
  local height = ds.get_frame_height(fileIndex, image_type_id, res_type, frame)
  local imageData = love.image.newImageData(width, height, "rgba8", data)
  return imageData
end

return gff
