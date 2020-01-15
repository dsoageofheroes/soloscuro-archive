local animate = {}
local private = {}

function animate.updateCollection(dt, graphics)
  for _,g in ipairs(graphics) do
    animate.update(dt, g)
  end
end

function animate.update(dt, graphic)
  if graphic.animation and graphic.timer then
    graphic.timer = graphic.timer + dt
    graphic.anim = graphic.animation[math.ceil(graphic.timer / graphic.animation.interval)]

    if graphic.timer > (#graphic.animation * graphic.animation.interval) then
      graphic.timer = nil
      graphic.anim = nil

      if graphic.animComplete then
        graphic:animComplete()
      end
    end
  end
end

return animate