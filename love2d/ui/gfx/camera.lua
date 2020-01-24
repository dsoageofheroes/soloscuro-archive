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

function private.isInView(graphic)

end

return camera