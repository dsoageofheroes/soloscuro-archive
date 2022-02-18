ticks = 0
move = 0
p0 = {}
function idle()
    local sol = soloscuro
    reg = sol.get_region()
    if (sol.in_combat() and (ticks % 30) == 0) then
        if (move == 0 and p0.move_left()) then
            move = 1
        elseif (p0.move_right()) then
            move = 0
        end
    end
    
    if (ticks > 900) then
        if (sol.in_combat()) then
            sol.fail("Slig should have defeated player.")
        else
            sol.exit()
        end
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
    gpl.clone(-269, 1, 30, 20, 6, 0)
    slig = reg.find("Slig")
    reg.enter_combat(reg, 20, 10)
    sol.game_loop()
end
