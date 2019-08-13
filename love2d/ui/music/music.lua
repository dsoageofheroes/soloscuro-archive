local music = require('dsmusic')
local ds = require('libds')

-- Soundfont file
local SF = "music.sf"

function music.init()
    dsmusic.music_init(SF)
end

function music.music_play_mem(midi_data)
    dsmusic.music_play_data(midi_data)
end

function music.stop()
    dsmusic.music_stop()
end

function music.shutdown()
    dsmusic.music_shutdown()
end

return music
