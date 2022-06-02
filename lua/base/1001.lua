-- Some doors don't have links...
function func1 (obj)
    gpl.request(2, obj, 1, 1) -- DOOR
end
-- Some camps don't have the camp option...
function func2 (obj)
    gpl.request(1, obj, 1, 0) -- CAMP
end
