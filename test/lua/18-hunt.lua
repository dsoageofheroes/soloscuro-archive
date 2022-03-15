ticks = 0
move = 0
p0 = {}
go_left = 0
num_cast = 0

function move_towards(me, enemy)
    dx = me.mapx - enemy.mapx;
    dy = me.mapy - enemy.mapy;

    if (dx == 0 and dy < 0) then
        --print("DOWN")
        me.move_down()
    elseif (dx == 0 and dy > 0) then
        --print("UP")
        me.move_up()
    elseif (dx < 0 and dy == 0) then
        --print("RIGHT")
        me.move_right()
    elseif (dx > 0 and dy == 0) then
        --print("LEFT")
        me.move_left()
    elseif (dx > 0 and dy > 0) then
        --print("UP LEFT")
        me.move_up_left()
    elseif (dx > 0 and dy < 0) then
        --print("DOWN LEFT")
        me.move_down_left()
    elseif (dx < 0 and dy < 0) then
        --print("DOWN RIGHT")
        me.move_down_right()
    elseif (dx < 0 and dy > 0) then
        --print("UP RIGHT")
        me.move_up_right()
    end
end

function idle()
    local sol = soloscuro
    reg = sol.get_region()
    if (p0.is_combat_turn() and (ticks % 30) == 0) then
        enemy = p0.get_closest_enemy()
        move_towards(p0, enemy)
    end

    if (ticks < 200) then
        if p0.in_combat() then
            sol.fail("Player should not be in combat yet.")
        end
    end

    if (ticks > 500 and ticks < 600) then
        if not p0.in_combat() then
            sol.fail("Player should be in combat.")
        end
    end

    if ticks > 1200 then
        if p0.in_combat() then
            sol.fail("Player should have defeated sligs.")
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
    p0.stats.str = 20
    p0.stats.dex = 22
    p0.name = "Test"
    p0.mapx = 25;
    p0.mapy = 10;
    p0.stats.hp = 140;
    --p0.set_class(0, 11) -- preserver
    p0.set_class(0, 9)  -- fighter
    p0.award_exp(90000)
    p0.give_ds1_item(3, 47, -30001)
    p0.give_ds1_item(10, 47, -30001)
    p0.give_ds1_item(2, 1, -1017) -- BOW
    p0.give_ds1_item(1, 62, -1070) -- arrows

    reg = sol.create_region()
    sol.set_region(reg)
    reg.set_tile_id(4, 83)
    sol.load_window("map")
    sol.load_window("narrate")
    sol.load_window("combat")
    gpl.clone(-269, 1, 45, 15, 6, 0)
    slig = reg.find("Slig")
    slig.hunt(p0);
    gpl.clone(-269, 1, 45, 15, 6, 0)
    slig = reg.find("Slig")
    slig.hunt(p0);
    gpl.clone(-269, 1, 45, 15, 6, 0)
    slig = reg.find("Slig")
    slig.hunt(p0);
    --reg.enter_combat(reg, 20, 10)
end
