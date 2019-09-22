return function(_value, _font, _x, _y, _w, _align)
  local self =
  {
    value = _value,
    font = _font,
    x = _x,
    y = _y,
    w = _w,
    align = _align or 'left'
  }

  return self
end