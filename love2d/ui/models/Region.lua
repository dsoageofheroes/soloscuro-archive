return function(_gff, filename)
  local region = {}

  region.tiles = _gff.loadTiles(filename)
  region.map = _gff.loadMap(filename, region.tiles)

  function region.shiftX(_x)
    for i,v in ipairs(region.map) do
      v.x = v.x + _x
    end
  end

  function region.shiftY(_y)
    for i,v in ipairs(region.map) do
      v.y = v.y + _y
    end
  end

  return region
end