local draw = {
  scale = 3,
  debug1 = '',
  debug2 = ''
}
local private = {}

local resolution

function draw.init(_config)
  resolution = _config.env.resolution
  draw.scale = _config.env.resolution.scale

  love.graphics.setDefaultFilter('nearest', 'nearest')
  private.resetWindow()
end

function draw.clickItemIn(graphics, x, y, button)
  local clicked = false

  for k,v in pairs(graphics) do
    if v.clicked and draw.isOverGraphic(v, x, y) then
      v:clicked(button)
      clicked = true
      break
    end
  end

  return clicked
end

function draw.graphic(graphic, xOffset, yOffset)
  local a = graphic.anim
  local h = graphic.hover
  local i = get(graphic.active) or 1

  -- Animations take precedence
  local image = graphic.timer and graphic.assets[a]

  if image then
    private.drawImage(image, graphic.x, graphic.y, graphic.aspect, xOffset, yOffset)
  else
    -- Use the base asset
    image = graphic.assets[i]
    private.drawImage(image, graphic.x, graphic.y, graphic.aspect, xOffset, yOffset)
    
    -- If we have a hover graphic, draw over the base
    if h then
      local inBox = draw.isMouseInBox(graphic.x, graphic.y, image:getWidth(), image:getHeight(), graphic.aspect)

      if inBox then
        private.drawImage(graphic.assets[h], graphic.x, graphic.y, graphic.aspect, xOffset, yOffset)
      end
    end
  end
end

function draw.collection(graphics)
  for _,graphic in pairs(graphics) do
    draw.graphic(graphic)
  end
end

function draw.absolute(image, x, y)
  love.graphics.draw(image, x, y, 0, draw.scale, draw.scale * 1.2)
end

function draw.text(text)
  love.graphics.setFont(get(text.font))

  if text.w then
    love.graphics.printf(
      get(text.value), 
      text.x * draw.scale, 
      text.y * draw.scale, 
      text.w * draw.scale, 
      text.align, 
      0, 
      draw.scale,
      draw.scale
    )
  else
    love.graphics.print(
      get(text.value), 
      text.x * draw.scale, 
      text.y * draw.scale, 
      0, 
      draw.scale,
      draw.scale
    )
  end
end

function draw.textCollection(texts)
  for k,v in pairs(texts) do
    if type(v) == 'table' then
      draw.text(v)
    end
  end
end

function private.drawImage(image, x, y, aspect, xOffset, yOffset)
  xOffset = xOffset or 0
  yOffset = yOffset or 0

  local xFinal = x * draw.scale + xOffset
  local yFinal = y * draw.scale * aspect + yOffset

  love.graphics.draw(image, xFinal, yFinal, 0, draw.scale, draw.scale * aspect)
end

function draw.isMouseInBox(x, y, width, height, aspect)
  local mx = love.mouse.getX()
  local my = love.mouse.getY()
  
  return private.isInBox(mx, my, x, y, width, height, aspect)
end

function draw.isOverGraphic(graphic, mx, my)
  local act = graphic['active'] or 1
  local w = graphic.assets[act]:getWidth()
  local h = graphic.assets[act]:getHeight()
  return private.isInBox(mx, my, graphic.x, graphic.y, w, h, graphic.aspect)
end

function private.isInBox(testX, testY, boxX, boxY, boxWidth, boxHeight, aspect)
  local westX = boxX * draw.scale
  local northY = boxY * draw.scale * aspect
  local eastX = (boxX + boxWidth) * draw.scale
  local southY = (boxY + boxHeight) * draw.scale * aspect

  return westX < testX and testX <= eastX and northY < testY and testY <= southY
end

function draw.scaleUp()
  draw.scale = draw.scale > 4 and 5 or draw.scale + 1
  private.resetWindow()
end

function draw.scaleDown()
  draw.scale = draw.scale < 2 and 1 or draw.scale - 1
  private.resetWindow()
end

function draw.debug()
  love.graphics.print(draw.debug1, 10 * draw.scale, 185 * draw.scale * 1.2)
  love.graphics.print(draw.debug2, 10 * draw.scale, 190 * draw.scale * 1.2)
end

function private.resetWindow()
  love.window.setMode(resolution.x, resolution.y)
  love.window.setFullscreen(resolution.fullscreen)
end

return draw