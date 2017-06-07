local function hexlify(data)
   local result = ""

   if data == nil then
      return result
   end

   for i=1, string.len(data) do
      local byte = string.byte(data, i)
      result = result .. string.format("%02x", byte)
   end
   return result
end

return hexlify
