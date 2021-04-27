dofile("./lua/player_input.lua")
dofile("./lua/settings.lua")

function init()
--[[
    --ds.run_browser() -- uncomment to just run the brower.
    -- after runnign the browser you probably should exit, unless you want to run the game.
    game_settings()
    --ds.load_region(42) -- right now only 42 works (there is some hard-coding.)

    ds.load_charsave(0, 15) -- load character from charsave.gff
    ds.load_charsave(1, 16) -- load character from charsave.gff

    ds.set_ignore_repeat(true) -- set to false if you want repeats

    e = soloscuro.load_player(0)
    print (e.name)
    e.mapx = 33
    print (e.mapx)
    print (e.mapy)

    print (e.stats.dex)
    print (e.stats.attack1.num_dice)
    print (e.stats.saves.paralysis)
    print (e.class1.current_xp)
    print (e.class2.current_xp)
    print (e.class3.current_xp)

    reg = soloscuro.create_region()
    reg.map_id = 1000
    print (reg.map_id)
    
    simple_gff = soloscuro.open_gff("test/data/simple.gff")
    -- The region has to be attacked to the file with tiles.
    reg.gff_file = simple_gff
    print (reg.gff_file)

    reg:set_tile_id(1, 142)
    for i=1,98 do
        for j=1,128 do
            reg:set_tile(i, j, 1)
        end
    end 

    soloscuro.set_region(reg)
    soloscuro.start_game()
    --]]

    return true -- returning true, means that lua has setup the game, don't do the default
                -- returning false, would allow the engine to proceed with normal startup
end

