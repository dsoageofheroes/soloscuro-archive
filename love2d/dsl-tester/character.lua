-- This represent a Playable Character (PC)

-- Here are the setters, right now we just have hp, but I can/will others if this looks good
character_set_functions = {
    ["hp"] = function (id, val) ds.set_hp(id, val) end
}

-- Here are the getters, right now we just have hp, but I can/will others if this looks good
character_get_functions = {
    ["hp"] = function (id) return ds.get_hp(id) end
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

function Character()
  local mt = {}
  mt.__index = function(t, k) return character_get(t, k) end
  mt.__newindex = function(t, j, k) return character_set(t, j, k) end
  local table = {}
  table.id = ds.create_character_id() -- Needed to start the id.  It should never be changed for the lifetime of the
                                      -- character.
  return setmetatable(table, mt)
end
