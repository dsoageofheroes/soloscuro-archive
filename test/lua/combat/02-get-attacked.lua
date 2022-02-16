ticks = 0
p0 = {}
function idle()
    local sol = soloscuro
    reg = sol.get_region()
    if (not sol.in_combat()) then
        sol.exit()
    end
    if ticks > 300 then
        sol.fail("Slig should have defeated player.")
    end
    ticks = ticks + 1
end

function init()
    local sol = soloscuro
    sol.set_quiet(false);
    p0 = sol.create_player(0)
    p0.stats.intel = 20
    p0.name = "Test"
    p0.mapx = 25;
    p0.mapy = 10;
    reg = sol.create_region()
    sol.set_region(reg)
    reg.set_tile_id(4, 83)
    sol.load_window("map")
    sol.load_window("narrate")
    sol.load_window("combat")
    gpl.clone(-269, 1, 30, 10, 6, 0)
    slig = reg.find("Slig")
    reg.enter_combat(reg, 20, 10)
    sol.game_loop()
end
