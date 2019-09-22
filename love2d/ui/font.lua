local private = {}
local font = 
{
  -- These are implemented as getter functions to allow memoized lazy loading
  dark = function() return private.get('202038FF', 'A6A6BEFF') end,
  light = function() return private.get('8A8AA2FF', '202038FF') end,
  black = function() return private.get('000000FF', 'A67171FF') end,
  red = function() return private.get('D74128FF', 'A6A6BEFF') end,
  yellow = function() return private.get('E7EB18FF', '202038FF') end,
  white = function() return private.get('D7D7E3FF', '202038FF') end,
}

local 
loadChar,
fontDivider,
smallFontImageData,
largeFontImageData,
glyphs

function font.init(_loadChar)
  loadChar = _loadChar

  private.loadAssets()
end

--[[ DEPRECATED

local function getFontDivider()
  if fontDivider == nil then
    fontDivider = love.image.newImageData(1,9)
    for i=1,9 do
      fontDivider:setPixel(0, i - 1, 0.1, 0.1, 0.1, 0.1)
    end
  end

  return fontDivider
end

local function buildFont(color, shadow)
  local sheetHeight = 9
  local sheetWidth = 0

  local chars = {}
  
  for i=0,255 do
    local char = loadChar(i, color, shadow)

    if char then
      table.insert(chars, char)

      sheetWidth = sheetWidth + char:getWidth()
    end
  end

  local fontSheet = love.image.newImageData(sheetWidth, sheetHeight)
  local divider = getFontDivider()
  local charX = 0
  local charY = 0

  for i=1,#chars do
    fontSheet:paste(divider, charX, charY, 0, 0, divider:getWidth(), divider:getHeight())
    fontSheet:paste(chars[i], charX + 1, charY, 0, 0, chars[i]:getWidth(), chars[i]:getHeight())
    charX = charX + chars[i]:getWidth() + 1
  end

  -- Drop in one last divider for the font image
  fontSheet:paste(divider, charX, charY, 0, 0, divider:getWidth(), divider:getHeight())

  local glyphs = [-[ !"#$%&'()*+,-./0123456789:;<=>?%ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{@}~^]-]
  local imageFont = love.graphics.newImageFont(fontSheet, glyphs)

  return imageFont
end
]]

local function buildFont(color, shadow)
  local w = smallFontImageData:getWidth()
  local h = smallFontImageData:getHeight()
  local fontImageData = love.image.newImageData(w, h)

  local cR, cG, cB, cA = private.hexToRGBA(color)
  local sR, sG, sB, sA = private.hexToRGBA(shadow)

  for i=0,w-1 do
    for j=0,h-1 do
      local r, g, b, a = smallFontImageData:getPixel(i, j)

      if r == 1 and a == 1 then
        fontImageData:setPixel(i, j, cR, cG, cB, cA)
      elseif r == 0 and a == 1 then
        fontImageData:setPixel(i, j, sR, sG, sB, sA)
      else
        fontImageData:setPixel(i, j, r, g, b, a)
      end
    end
  end

  return love.graphics.newImageFont(fontImageData, glyphs)
end

function private.get(color, shadow)
  local key = color .. shadow

  if not private[key] then
    private[key] = buildFont(color, shadow)
  else
  end

  return private[key]
end

function private.loadAssets()
  for line in love.filesystem.lines("assets/font/glyphs.txt") do
    glyphs = line
  end

  smallFontImageData = love.image.newImageData('assets/font/smallfont.png')
  largeFontImageData = love.image.newImageData('assets/font/largefont.png')
end

function private.hexToRGBA(hex)
  return 
    tonumber("0x"..hex:sub(1,2)) / 0xFF, 
    tonumber("0x"..hex:sub(3,4)) / 0xFF, 
    tonumber("0x"..hex:sub(5,6)) / 0xFF, 
    tonumber("0x"..hex:sub(7,8)) / 0xFF
end

return font