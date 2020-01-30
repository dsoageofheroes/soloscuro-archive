require 'dsl/character'
local charTester = {}
-- This loads the C library and binds is to a 'ds' object
local ds = require 'libds'
dsl = {}

function charTester.run()
    -- Lets create four characters:
    player1 = Character(0)
    player2 = Character(1)
    player3 = Character(2)
    player4 = Character(3)

    -- Now print out thier ids:
    print ("Player 1's id = " .. player1.id)
    print ("Player 2's id = " .. player2.id)
    print ("Player 3's id = " .. player3.id)
    print ("Player 4's id = " .. player4.id)

    player1.current_xp = 500;
    player1.high_xp = 0;
    player1.base_hp = 1;
    player1.high_hp = 2;
    player1.base_psp = 3;
    player1.legal_class = 5;
    player1.race = 6;
    player1.gender = 7;
    player1.alignment = 8;
    player1.str = 9;
    player1.dex = 10;
    player1.con = 11;
    player1.int = 12;
    player1.wis = 13;
    player1.cha = 14;
    player1.class0 = 15;
    player1.class1 = 16;
    player1.class2 = 17;
    player1.level0 = 18;
    player1.level1 = 19;
    player1.level2 = 20;
    player1.base_ac = 21;
    player1.base_move = 22;
    player1.magic_resistance = 23;
    player1.num_blows = 24;
    player1.num_attacks0 = 25;
    player1.num_attacks1 = 26;
    player1.num_attacks2 = 27;
    player1.num_dice0 = 28;
    player1.num_dice1 = 29;
    player1.num_dice2 = 30;
    player1.num_sides0 = 31;
    player1.num_sides1 = 32;
    player1.num_sides2 = 33;
    player1.num_bonuses0 = 34;
    player1.num_bonuses1 = 35;
    player1.num_bonuses2 = 36;
    player1.paral = 37;
    player1.wand = 38;
    player1.petr = 39;
    player1.breath = 40;
    player1.spell = 41;
    player1.allegiance = 42;
    player1.size = 43;
    player1.spell_group = 44;
    player1.high_level0 = 45;
    player1.high_level1 = 46;
    player1.high_level2 = 47;
    player1.sound_fx = 48;
    player1.attack_sound = 49;
    player1.psi_group = 50;
    player1.palette = 51;

    print ("Player 1's current_xp = " .. player1.current_xp)
    print ("Player 1's high_xp = " .. player1.high_xp)
    print ("Player 1's base_hp = " .. player1.base_hp)
    print ("Player 1's high_hp = " .. player1.high_hp)
    print ("Player 1's base_psp = " .. player1.base_psp)
    print ("Player 1's legal_class = " .. player1.legal_class)
    print ("Player 1's race = " .. player1.race)
    print ("Player 1's str = " .. player1.str)
    print ("Player 1's dex = " .. player1.dex)
    print ("Player 1's con = " .. player1.con)
    print ("Player 1's int = " .. player1.int)
    print ("Player 1's wis = " .. player1.wis)
    print ("Player 1's cha = " .. player1.cha)
    print ("Player 1's class0 = " .. player1.class0)
    print ("Player 1's class1 = " .. player1.class1)
    print ("Player 1's class2 = " .. player1.class2)
    print ("Player 1's level0 = " .. player1.level0)
    print ("Player 1's level1 = " .. player1.level1)
    print ("Player 1's level2 = " .. player1.level2)
    print ("Player 1's base_ac = " .. player1.base_ac)
    print ("Player 1's base_move = " .. player1.base_move)
    print ("Player 1's magic_resistance = " .. player1.magic_resistance)
    print ("Player 1's num_blows = " .. player1.num_blows)
    print ("Player 1's num_attacks0 = " .. player1.num_attacks0)
    print ("Player 1's num_attacks1 = " .. player1.num_attacks1)
    print ("Player 1's num_attacks2 = " .. player1.num_attacks2)
    print ("Player 1's num_dice0 = " .. player1.num_dice0)
    print ("Player 1's num_dice1 = " .. player1.num_dice1)
    print ("Player 1's num_dice2 = " .. player1.num_dice2)
    print ("Player 1's num_sides0 = " .. player1.num_sides0)
    print ("Player 1's num_sides1 = " .. player1.num_sides1)
    print ("Player 1's num_sides2 = " .. player1.num_sides2)
    print ("Player 1's num_bonuses0 = " .. player1.num_bonuses0)
    print ("Player 1's num_bonuses1 = " .. player1.num_bonuses1)
    print ("Player 1's num_bonuses2 = " .. player1.num_bonuses2)
    print ("Player 1's paral = " .. player1.paral)
    print ("Player 1's petr = " .. player1.petr)
    print ("Player 1's breath = " .. player1.breath)
    print ("Player 1's spell = " .. player1.spell)
    print ("Player 1's allegiance = " .. player1.allegiance)
    print ("Player 1's size = " .. player1.size)
    print ("Player 1's spell_group = " .. player1.spell_group)
    print ("Player 1's high_level0 = " .. player1.high_level0)
    print ("Player 1's high_level1 = " .. player1.high_level1)
    print ("Player 1's high_level2 = " .. player1.high_level2)
    print ("Player 1's sound_fx = " .. player1.sound_fx)
    print ("Player 1's attack_sound = " .. player1.attack_sound)
    print ("Player 1's psi_group = " .. player1.psi_group)
    print ("Player 1's palette = " .. player1.palette)

    -- The following causes a LUA error because I haven't defined str yet:
    --player1.str = 5
    print ("END CHARACTER TEST")
end

return charTester
