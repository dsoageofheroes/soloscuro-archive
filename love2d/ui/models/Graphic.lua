return function(_assetFrames, _x, _y)
  local graphic = 
  {
    assets = type(_assetFrames) == 'table' and _assetFrames or {_assetFrames},
    x = _x or 0,
    y = _y or 0,
    aspect = 1.2,
  }

  function graphic:getWidth()
    local w = 0
    local image = self.active and self.assets[self.active] or self.assets[1]

    if (image) then
      w = image:getWidth()
    end

    return w
  end

  function graphic:getHeight()
    local h = 0
    local image = self.active and self.assets[self.active] or self.assets[1]

    if (image) then
      h = image:getHeight()
    end

    return h
  end

  function graphic:setAspect(aspect)
    self.aspect = aspect
  end

  function graphic:setActive(index)
    self.active = index
    return self
  end

  function graphic:setHover(index)
    self.hover = index
    return self
  end

  function graphic:setClicked(event)
    self.clicked = event
    return self
  end

  function graphic:animate(_animation, _animComplete)
    self.animation = _animation
    self.animComplete = _animComplete
    return self
  end

  return graphic
end