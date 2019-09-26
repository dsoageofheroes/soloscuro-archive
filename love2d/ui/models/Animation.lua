return function(...)
  local animation = {...}

  animation.interval = .1

  function animation:setInterval(_interval)
    self.interval = _interval
    return self
  end

  return animation
end