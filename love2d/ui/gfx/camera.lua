local camera = 
{
  x = 0,
  y = 0
}
local private = {}

local draw

function camera.init(_draw)
  draw = _draw
end

function camera.show(collection)
  for _,graphic in ipairs(collection) do
    draw.graphic(graphic)
  end
end

return camera