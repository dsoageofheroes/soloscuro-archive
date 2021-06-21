function init()
    soloscuro.load_charsave(0, 3) -- load character from charsave.gff
    -- soloscuro.load_region(42)
    print (soloscuro.change_region(42))
    --print (e.name)
    soloscuro.load_window("map")
    soloscuro.load_window("narrate")
    soloscuro.load_window("combat")
    soloscuro.game_loop()
end
