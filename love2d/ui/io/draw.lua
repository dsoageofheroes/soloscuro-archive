local draw = {
  debug1 = '',
  debug2 = ''
}
local private = {}

local scaleFactor = 3
local aspectCorrection = 1.2
local resolution

function draw.init(_config)
  resolution = _config.env.resolution
  scaleFactor = _config.env.resolution.scale
  aspectCorrection = _config.env.resolution.aspect

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

function draw.graphic(graphic, noAspect)
  local a = graphic.anim
  local h = graphic.hover
  local i = get(graphic.active) or 1

  -- Animations take precedence
  local image = graphic.timer and graphic.assets[a]

  if image then
    private.drawImage(image, graphic.x, graphic.y)
  else
    -- Use the base asset
    image = graphic.assets[i]
    private.drawImage(image, graphic.x, graphic.y, noAspect)
    
    -- If we have a hover graphic, draw over the base
    if h then
      local inBox = draw.isMouseInBox(graphic.x, graphic.y, image:getWidth(), image:getHeight())

      if inBox then
        private.drawImage(graphic.assets[h], graphic.x, graphic.y)
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
  love.graphics.draw(image, x, y, 0, scaleFactor, scaleFactor * aspectCorrection)
end

function draw.text(text)
  love.graphics.setFont(get(text.font))

  if text.w then
    love.graphics.printf(
      get(text.value), 
      text.x * scaleFactor, 
      text.y * scaleFactor, 
      text.w * scaleFactor, 
      text.align, 
      0, 
      scaleFactor,
      scaleFactor
    )
  else
    love.graphics.print(
      get(text.value), 
      text.x * scaleFactor, 
      text.y * scaleFactor, 
      0, 
      scaleFactor,
      scaleFactor
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

function private.drawImage(image, x, y, noAspect, xOffset, yOffset)
  xOffset = xOffset or 0
  yOffset = yOffset or 0

  local aspect = noAspect and 1 or aspectCorrection
  local xFinal = x * scaleFactor + xOffset
  local yFinal = y * scaleFactor * aspect + yOffset

  love.graphics.draw(image, xFinal, yFinal, 0, scaleFactor, scaleFactor * aspect)
end

function draw.isMouseInBox(x, y, width, height)
  local mx = love.mouse.getX()
  local my = love.mouse.getY()
  
  return private.isInBox(mx, my, x, y, width, height, true)
end

function draw.isOverGraphic(graphic, mx, my)
  local act = graphic['active'] or 1
  local w = graphic.assets[act]:getWidth()
  local h = graphic.assets[act]:getHeight()
  return private.isInBox(mx, my, graphic.x, graphic.y, w, h, true)
end

function private.isInBox(testX, testY, boxX, boxY, boxWidth, boxHeight, correctAspect)
  local westX = boxX * scaleFactor
  local northY = boxY * scaleFactor * (correctAspect and aspectCorrection or 1)
  local eastX = (boxX + boxWidth) * scaleFactor
  local southY = (boxY + boxHeight) * scaleFactor * (correctAspect and aspectCorrection or 1)

  return westX < testX and testX <= eastX and northY < testY and testY <= southY
end

function draw.scaleUp()
  scaleFactor = scaleFactor > 4 and 5 or scaleFactor + 1
  private.resetWindow()
end

function draw.scaleDown()
  scaleFactor = scaleFactor < 2 and 1 or scaleFactor - 1
  private.resetWindow()
end

function draw.debug()
  love.graphics.print(draw.debug1, 10 * scaleFactor, 185 * scaleFactor * aspectCorrection)
  love.graphics.print(draw.debug2, 10 * scaleFactor, 190 * scaleFactor * aspectCorrection)
end

function private.resetWindow()
  -- love.window.setMode(1920, 1080)
  love.window.setMode(resolution.x, resolution.y) -- * aspectCorrection)
end

return draw