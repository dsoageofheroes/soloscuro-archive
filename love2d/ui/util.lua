function get(value)
  return type(value) == 'function' and value() or value
end

function addRange(target, addedRange)
  if type(target) ~= 'table' or type(addedRange) ~= 'table' then
    return nil
  end

  for k,v in pairs(addedRange) do
    table.insert(target, v)
  end
end