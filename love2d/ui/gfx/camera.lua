local camera = {}
local draw
local private = 
{
  xOffset = 0,
  yOffset = 0,
}

function camera.init(_config, _draw)
  draw = _draw

  camera.width = _config.env.resolution.x
  camera.height = _config.env.resolution.y
end

function camera.update(dt)
  local x = love.mouse.getX()
  local y = love.mouse.getY()

  -- Move left
  if x < 6 * draw.scale then
    camera.moveX(440 * dt * draw.scale)
  end

  -- Move right
  if x > camera.width - (6 * draw.scale) then
    camera.moveX(-440 * dt * draw.scale)
  end

  -- Move up
  if y < 6 * draw.scale then
    camera.moveY(440 * dt * draw.scale)
  end

  -- Move down
  if y > camera.height - (6 * draw.scale) then
    camera.moveY(-440 * dt * draw.scale)
  end
end

function camera.show(collection)
  for _,graphic in ipairs(collection) do
    local left = graphic.x * draw.scale
    local top = graphic.y * draw.scale * graphic.aspect
    local right = left + graphic:getWidth() * draw.scale
    local bottom = top + graphic:getHeight() * draw.scale * graphic.aspect

    if ((left + private.xOffset) <= camera.width and (right + private.xOffset) >= 0 and
        (top + private.yOffset) <= camera.height and (bottom + private.yOffset) >= 0
      ) then
      draw.graphic(graphic, private.xOffset, private.yOffset)
    end
  end
end

function camera.moveX(_x)
  private.xOffset = private.xOffset + _x
end

function camera.moveY(_y)
  private.yOffset = private.yOffset + _y
end

function camera.setX(_x)
  private.xOffset = _x
end

function camera.setY(_y)
  private.yOffset = _y
end

return camera