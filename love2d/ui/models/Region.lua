DSLDATA_GFF_INDEX = 2

return function(_gff, filename)
  local region = {}
  -- This needs to be changed via what region it is!
  paletteId = ds.gff_get_palette_id(DSLDATA_GFF_INDEX, 41)

  region.tiles = _gff.loadTiles(filename, paletteId)
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
