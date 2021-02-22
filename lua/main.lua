function init()
    ds.load_charsave(0, 15) -- load character from charsave.gff
    ds.load_charsave(1, 16) -- load character from charsave.gff

    ds.set_ignore_repeat(true) -- set to false if you want repeats
    ds.load_region(42) -- right now only 42 works (there is some hard-coding.)

    return true -- returning true, means that lua has setup the game, don't do the default
                -- returning false, would allow the engine to proceed with normal startup
end

dofile("./lua/player_input.lua")
dofile("./lua/settings.lua")
