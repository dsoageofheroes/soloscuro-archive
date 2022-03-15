ticks = 0
move = 0
p0 = {}
go_right = 0
function idle()
    local sol = soloscuro
    reg = sol.get_region()
    if (sol.in_combat() and (ticks % 30) == 0) then
        if (go_right == 0 and p0.move_down()) then
            go_right = 1
        else
            p0.move_right()
        end
    end
    
    if (ticks > 800) then
        if (sol.in_combat()) then
            sol.fail("Player should have defeated slig.")
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
    p0.stats.hp = 40;
    p0.set_class(0, 9)
    p0.award_exp(900000)
    p0.give_ds1_item(3, 47, -30001)
    p0.give_ds1_item(10, 47, -30001)
    reg = sol.create_region()
    sol.set_region(reg)
    reg.set_tile_id(4, 83)
    sol.load_window("map")
    sol.load_window("narrate")
    sol.load_window("combat")
    gpl.clone(-269, 1, 30, 10, 6, 0)
    slig = reg.find("Slig")
    reg.enter_combat(reg, 20, 10)
end
