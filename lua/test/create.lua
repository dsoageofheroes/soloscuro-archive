function init()
    local sol = soloscuro
    --sol.load_charsave(0, 17) -- load character from charsave.gff
    --player = sol.load_player(0)
    p0 = sol.create_player(0)
    print (p0.stats.dex)
    p1 = sol.load_player(0)
    p0.stats.dex = 20
    p0.name = "Test"
    print (p1.stats.dex)
    print (p1.name)
    reg = sol.create_region()
    sol.set_region(reg)
    reg.set_tile_id(reg.test())
    sol.load_window("map")
    sol.load_window("narrate")
    sol.load_window("combat")
    sol.game_loop()
end
