function init()
    soloscuro.load_charsave(0, 15) -- load character from charsave.gff
    soloscuro.load_charsave(1, 16) -- load character from charsave.gff
    soloscuro.set_ignore_repeat(true) -- set to false if you want repeats
    e = soloscuro.load_player(0)
    print (e.name)
    soloscuro.load_window("view")
end
