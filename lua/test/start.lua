function init()
    local sol = soloscuro
    sol.load_charsave(0, 17) -- load character from charsave.gff
    -- soloscuro.load_region(42)
    print (sol.change_region(42))
    --print (e.name)
    sol.load_window("map")
    sol.load_window("narrate")
    sol.load_window("combat")
    sol.game_loop()
end
