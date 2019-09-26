return function(_value, _font, _x, _y)
  local text =
  {
    value = _value,
    font = _font,
    x = _x,
    y = _y,
  }

  function text:box(_w, _align)
    self.w = _w
    self.align = _align or 'left'

    return self
  end

  return text
end