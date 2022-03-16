function idle()
    local sol = soloscuro
    reg = sol.get_region()
    slig = reg.find("Slig")
    if (slig == nil) then
        return
    end
    p0 = sol.get_player(0)
    p0.cast(slig, sol.WIZ_MAGIC_MISSILE)
end
function init()
    local sol = soloscuro
    sol.set_quiet(false);
    p0 = sol.create_player(0)
    p0.stats.intel = 20
    p0.name = "Test"
    p0.mapx = 20;
    p0.mapy = 10;
    reg = sol.create_region()
    sol.set_region(reg)
    reg.set_tile_id(4, 83)
    sol.load_window("map")
    sol.load_window("narrate")
    sol.load_window("combat")
    gpl.clone(-269, 3, 40, 10, 6, 0)
    slig = reg.find("Slig")
    print(p0.name)
    print(slig.name)
    print(sol.WIZ_MAGIC_MISSILE)
    p0.cast(slig, sol.WIZ_MAGIC_MISSILE)
end
