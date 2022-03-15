ticks = 0
p0 = {}
function idle()
    local sol = soloscuro
    reg = sol.get_region()
    if (not p0.in_combat()) then
        sol.exit()
    end
    if ticks > 30 then
        sol.fail("A single combatant should not be in a combat zone after a round.")
    end
    ticks = ticks + 1
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
    reg.enter_combat(reg, 20, 10)
end
