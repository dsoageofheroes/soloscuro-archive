function keydown(num)
end

function keyup(num)
end

function idle()
end

function init()
    local sol = soloscuro
    sol.set_quiet(false);
    --sol.load_charsave(0, 17) -- load character from charsave.gff
    p0 = sol.create_player(0)
    p0.name = "Slad"
    p0.mapx = 30
    p0.mapy = 12
    p0.stats.intel = 20
    p0.stats.str = 20
    p0.stats.dex = 20
    p0.stats.con = 20
    p0.set_class(0, 9)
    p0.award_exp(900000)
    p0.give_ds1_item(3, 47, -30001)
    p0.give_ds1_item(10, 47, -30001)
    -- soloscuro.load_region(42)
    print (sol.change_region(42))
    --print (e.name)
    sol.load_window("map")
    sol.load_window("narrate")
    sol.load_window("combat")
    reg = sol.get_region()
    reg.run_mas()
end
