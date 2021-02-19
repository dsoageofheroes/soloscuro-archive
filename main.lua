function init()
    --ds.set_quiet(1) -- uncomment to place into quiet mode
    ds.load_charsave(0, 15) -- load character from charsave.gff
    ds.load_charsave(1, 16) -- load character from charsave.gff

    ds.set_ignore_repeat(true) -- sat to false if you want repeats
    ds.load_region(42) -- right now only 42 works (there is some hard-coding.)
    ds.set_player_frame_delay(7) -- set how many delay ticks between frame changes of a player sprite. (default: 7)
    ds.set_player_move(10) -- set how many ticks to move one slot. (default: 10)

    return true -- returning true, means that lua has setup the game, don't do the default
                -- returning false, would allow the engine to proceed with normal startup
end

dofile("player_input.lua")
