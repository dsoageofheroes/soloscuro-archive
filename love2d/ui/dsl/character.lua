-- This represent a Playable Character (PC)
local ds = require 'libds'

-- Here are the setters.
character_set_functions = {
    ["current_xp"] = function (id, val) ds.set_char_current_xp(id, val) end,
    ["high_xp"] = function (id, val) ds.set_char_high_xp(id, val) end,
    ["base_hp"] = function (id, val) ds.set_char_base_hp(id, val) end,
    ["high_hp"] = function (id, val) ds.set_char_high_hp(id, val) end,
    ["base_psp"] = function (id, val) ds.set_char_base_psp(id, val) end,
    ["legal_class"] = function (id, val) ds.set_char_legal_class(id, val) end,
    ["race"] = function (id, val) ds.set_char_race(id, val) end,
    ["gender"] = function (id, val) ds.set_char_gender(id, val) end,
    ["alignment"] = function (id, val) ds.set_char_alignment(id, val) end,
    ["str"] = function (id, val) ds.set_char_str(id, val) end,
    ["dex"] = function (id, val) ds.set_char_dex(id, val) end,
    ["con"] = function (id, val) ds.set_char_con(id, val) end,
    ["int"] = function (id, val) ds.set_char_int(id, val) end,
    ["wis"] = function (id, val) ds.set_char_wis(id, val) end,
    ["cha"] = function (id, val) ds.set_char_cha(id, val) end,
    ["class0"] = function (id, val) ds.set_char_class0(id, val) end,
    ["class1"] = function (id, val) ds.set_char_class1(id, val) end,
    ["class2"] = function (id, val) ds.set_char_class2(id, val) end,
    ["level0"] = function (id, val) ds.set_char_level0(id, val) end,
    ["level1"] = function (id, val) ds.set_char_level1(id, val) end,
    ["level2"] = function (id, val) ds.set_char_level2(id, val) end,
    ["base_ac"] = function (id, val) ds.set_char_base_ac(id, val) end,
    ["base_move"] = function (id, val) ds.set_char_base_move(id, val) end,
    ["magic_resistance"] = function (id, val) ds.set_char_magic_resistance(id, val) end,
    ["num_blows"] = function (id, val) ds.set_char_num_blows(id, val) end,
    ["num_attacks0"] = function (id, val) ds.set_char_num_attacks0(id, val) end,
    ["num_attacks1"] = function (id, val) ds.set_char_num_attacks1(id, val) end,
    ["num_attacks2"] = function (id, val) ds.set_char_num_attacks2(id, val) end,
    ["num_dice0"] = function (id, val) ds.set_char_num_dice0(id, val) end,
    ["num_dice1"] = function (id, val) ds.set_char_num_dice1(id, val) end,
    ["num_dice2"] = function (id, val) ds.set_char_num_dice2(id, val) end,
    ["num_sides0"] = function (id, val) ds.set_char_num_sides0(id, val) end,
    ["num_sides1"] = function (id, val) ds.set_char_num_sides1(id, val) end,
    ["num_sides2"] = function (id, val) ds.set_char_num_sides2(id, val) end,
    ["num_bonuses0"] = function (id, val) ds.set_char_num_bonuses0(id, val) end,
    ["num_bonuses1"] = function (id, val) ds.set_char_num_bonuses1(id, val) end,
    ["num_bonuses2"] = function (id, val) ds.set_char_num_bonuses2(id, val) end,
    ["paral"] = function (id, val) ds.set_char_paral(id, val) end,
    ["wand"] = function (id, val) ds.set_char_wand(id, val) end,
    ["petr"] = function (id, val) ds.set_char_petr(id, val) end,
    ["breath"] = function (id, val) ds.set_char_breath(id, val) end,
    ["spell"] = function (id, val) ds.set_char_spell(id, val) end,
    ["allegiance"] = function (id, val) ds.set_char_allegiance(id, val) end,
    ["size"] = function (id, val) ds.set_char_size(id, val) end,
    ["spell_group"] = function (id, val) ds.set_char_spell_group(id, val) end,
    ["high_level0"] = function (id, val) ds.set_char_high_level0(id, val) end,
    ["high_level1"] = function (id, val) ds.set_char_high_level1(id, val) end,
    ["high_level2"] = function (id, val) ds.set_char_high_level2(id, val) end,
    ["sound_fx"] = function (id, val) ds.set_char_sound_fx(id, val) end,
    ["attack_sound"] = function (id, val) ds.set_char_attack_sound(id, val) end,
    ["psi_group"] = function (id, val) ds.set_char_psi_group(id, val) end,
    ["palette"] = function (id, val) ds.set_char_palette(id, val) end
}

-- Here are the getters.
character_get_functions = {
    ["current_xp"] = function (id) return ds.get_char_current_xp(id) end,
    ["high_xp"] = function (id) return ds.get_char_high_xp(id) end,
    ["base_hp"] = function (id) return ds.get_char_base_hp(id) end,
    ["high_hp"] = function (id) return ds.get_char_high_hp(id) end,
    ["base_psp"] = function (id) return ds.get_char_base_psp(id) end,
    ["legal_class"] = function (id) return ds.get_char_legal_class(id) end,
    ["race"] = function (id) return ds.get_char_race(id) end,
    ["gender"] = function (id) return ds.get_char_gender(id) end,
    ["alignment"] = function (id) return ds.get_char_alignment(id) end,
    ["str"] = function (id) return ds.get_char_str(id) end,
    ["dex"] = function (id) return ds.get_char_dex(id) end,
    ["con"] = function (id) return ds.get_char_con(id) end,
    ["int"] = function (id) return ds.get_char_int(id) end,
    ["wis"] = function (id) return ds.get_char_wis(id) end,
    ["cha"] = function (id) return ds.get_char_cha(id) end,
    ["class0"] = function (id) return ds.get_char_class0(id) end,
    ["class1"] = function (id) return ds.get_char_class1(id) end,
    ["class2"] = function (id) return ds.get_char_class2(id) end,
    ["level0"] = function (id) return ds.get_char_level0(id) end,
    ["level1"] = function (id) return ds.get_char_level1(id) end,
    ["level2"] = function (id) return ds.get_char_level2(id) end,
    ["base_ac"] = function (id) return ds.get_char_base_ac(id) end,
    ["base_move"] = function (id) return ds.get_char_base_move(id) end,
    ["magic_resistance"] = function (id) return ds.get_char_magic_resistance(id) end,
    ["num_blows"] = function (id) return ds.get_char_num_blows(id) end,
    ["num_attacks0"] = function (id) return ds.get_char_num_attacks0(id) end,
    ["num_attacks1"] = function (id) return ds.get_char_num_attacks1(id) end,
    ["num_attacks2"] = function (id) return ds.get_char_num_attacks2(id) end,
    ["num_dice0"] = function (id) return ds.get_char_num_dice0(id) end,
    ["num_dice1"] = function (id) return ds.get_char_num_dice1(id) end,
    ["num_dice2"] = function (id) return ds.get_char_num_dice2(id) end,
    ["num_sides0"] = function (id) return ds.get_char_num_sides0(id) end,
    ["num_sides1"] = function (id) return ds.get_char_num_sides1(id) end,
    ["num_sides2"] = function (id) return ds.get_char_num_sides2(id) end,
    ["num_bonuses0"] = function (id) return ds.get_char_num_bonuses0(id) end,
    ["num_bonuses1"] = function (id) return ds.get_char_num_bonuses1(id) end,
    ["num_bonuses2"] = function (id) return ds.get_char_num_bonuses2(id) end,
    ["paral"] = function (id) return ds.get_char_paral(id) end,
    ["wand"] = function (id) return ds.get_char_wand(id) end,
    ["petr"] = function (id) return ds.get_char_petr(id) end,
    ["breath"] = function (id) return ds.get_char_breath(id) end,
    ["spell"] = function (id) return ds.get_char_spell(id) end,
    ["allegiance"] = function (id) return ds.get_char_allegiance(id) end,
    ["size"] = function (id) return ds.get_char_size(id) end,
    ["spell_group"] = function (id) return ds.get_char_spell_group(id) end,
    ["high_level0"] = function (id) return ds.get_char_high_level0(id) end,
    ["high_level1"] = function (id) return ds.get_char_high_level1(id) end,
    ["high_level2"] = function (id) return ds.get_char_high_level2(id) end,
    ["sound_fx"] = function (id) return ds.get_char_sound_fx(id) end,
    ["attack_sound"] = function (id) return ds.get_char_attack_sound(id) end,
    ["psi_group"] = function (id) return ds.get_char_psi_group(id) end,
    ["palette"] = function (id) return ds.get_char_palette(id) end
}

-- This is the function called when we do a character.<index> = <value>
function character_set(t, index, value)
    -- We don't allow the changing of ID, that MUST be managed in libds.
    if index == "id" then
        print("ERROR: attempt to change a character's ID!");
        return
    end
    character_set_functions[index](t.id, value)
end

-- This is the function called when we do a player1.<index>
function character_get(t, index)
    return character_get_functions[index](t.id)
end

function Character(id)
  if (ds.valid_character_id(id) < 1) then
      return nil;
  end
  local mt = {}
  mt.__index = function(t, k) return character_get(t, k) end
  mt.__newindex = function(t, j, k) return character_set(t, j, k) end
  local table = {}
  table.id = id -- Needed to start the id.  It should never be changed for the lifetime of the
                -- character.
  return setmetatable(table, mt)
end
