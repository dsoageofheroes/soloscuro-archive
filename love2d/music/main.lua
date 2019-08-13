fluidsyth = require 'dsmusic'

function love.load()
  love.graphics.setDefaultFilter('nearest', 'nearest')

end

function love.keypressed( key )
    if key == "down" then
        love.graphics.print("Pressed down ", 20, 10)
        fluidsynth.music_init("")
        fluidsynth.music_play("")
        fluidsynth.music_shutdown()
    end
    if key == "up" then
    end
    if key == "right" then
    end
    if key == "left" then
    end
end

function love.update(dt)
end

function love.draw()
    love.graphics.print("Enter (relative) filename of MIDI file ", 10, 10)
    

end
