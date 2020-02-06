DSObject = {}

function DSObject:id()
    print "ID CALL"
end

function ds1combatset(t, index, value)
    print("Set member " .. index .. " to " .. value)
    if index == "id" then
        rawset(t, index, value)
    end
end

function ds1combatget(t, index)
    print("Getting " .. index)
    --print("id = " .. t.id)
    return 6
end

function DS1Combat()
  local mt = {}
  mt.__index = function(t, k) return ds1combatget(t, k) end
  mt.__newindex = function(t, j, k) return ds1combatset(t, j, k) end
  local table = {}
  table.id = -1
  return setmetatable(table, mt)
end
