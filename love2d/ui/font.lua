local private = {}
local font = 
{
  -- Hardcoded glyph arrangement - keep the length exactly as-is
  glyphs = [[ !"#$%&'()*+,-./0123456789:;<=>?%ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{Ɵ}~ƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟƟ]],

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
fontPatch,
moreFontSheet

function font.init(_loadChar, _fontPatch)
  loadChar = _loadChar
  fontPatch = _fontPatch

  private.loadAssets()
  private.buildFontSheet()
end

function private.buildFontSheet()
  local sheetHeight = 9
  local sheetWidth = 1

  local chars = {}
  
  for i=0,255 do
    local char = loadChar(i)

    if char then
      table.insert(chars, private.patchChar(fontPatch[i], char))

      sheetWidth = sheetWidth + char:getWidth() + 1
    end
  end

  local fontSheet = love.image.newImageData(sheetWidth + moreFontSheet:getWidth(), sheetHeight)
  local divider = love.image.newImageData(1,9)
  local charX = 0
  local charY = 0

  for i=1,9 do
    divider:setPixel(0, i - 1, 0.1, 0.1, 0.1, 0.1)
  end

  for i=1,#chars do
    fontSheet:paste(divider, charX, charY, 0, 0, divider:getWidth(), divider:getHeight())
    fontSheet:paste(chars[i], charX + 1, charY, 0, 0, chars[i]:getWidth(), chars[i]:getHeight())
    charX = charX + chars[i]:getWidth() + 1
  end

  -- Add in the additional chars
  fontSheet:paste(moreFontSheet, charX, charY, 0, 0, moreFontSheet:getWidth(), moreFontSheet:getHeight())

  private.fontSheet = fontSheet
end

function private.patchChar(patch, char)
  local w = patch and patch.w or char:getWidth()
  local h = patch and patch.h or char:getHeight()
  local newChar = love.image.newImageData(w, h)

  for x=0,w-1 do
    for y=0,h-1 do
      local r,g,b,a = char:getPixel((patch and patch.x or 0) + x, y)
      r = r > .5 and 1 or 0 
      g = g > .5 and 1 or 0
      b = b > .5 and 1 or 0
      a = a > .5 and 1 or 0
      newChar:setPixel(x,y,r,g,b,a)
    end
  end

  if patch then
    for i,v in ipairs(patch) do
      newChar:setPixel(unpack(v))
    end
  end

  return newChar
end

function private.colorize(color, shadow)
  local newFont = private.fontSheet:clone()
  local cR,cG,cB,cA = private.hexToRGBA(color)
  local sR,sG,sB,sA = private.hexToRGBA(shadow)

  for x=0,newFont:getWidth()-1 do
    for y=0,newFont:getHeight()-1 do
      local r,g,b,a = private.fontSheet:getPixel(x,y)

      if a == 1 and r == 1 then
        newFont:setPixel(x,y,cR,cG,cB,cA)
      elseif a == 1 and r == 0 then
        newFont:setPixel(x,y,sR,sG,sB,sA)
      else
        newFont:setPixel(x,y,r,g,b,a)
      end
    end
  end

  return love.graphics.newImageFont(newFont, font.glyphs)
end

function private.get(color, shadow)
  local key = color .. shadow

  if not private[key] then
    private[key] = private.colorize(color, shadow)
  end

  return private[key]
end

function private.loadAssets()
  for line in love.filesystem.lines("assets/font/more_glyphs.txt") do
    font.glyphs = font.glyphs..line
  end

  moreFontSheet = love.image.newImageData('assets/font/more_font.png')
end

function private.hexToRGBA(hex)
  return 
    tonumber("0x"..hex:sub(1,2)) / 0xFF, 
    tonumber("0x"..hex:sub(3,4)) / 0xFF, 
    tonumber("0x"..hex:sub(5,6)) / 0xFF, 
    tonumber("0x"..hex:sub(7,8)) / 0xFF
end

return font