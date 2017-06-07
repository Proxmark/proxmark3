local cmds = require('commands')
local hexlify = require('hexlify')
local reader = require('read14a')

local Emulator = {
   _VERSION = 'emulator.lua 0.1.0',
   _DESCRIPTION = 'emulator memory interface'
}

function Emulator.set_mem (data, clear_first)
   if clear_first then
      -- Clear out the emulator memory first
      local emuMemclearCmd = Command:new{cmd = cmds.CMD_MIFARE_EML_MEMCLR,
                                         arg1 = 0, arg2 = 0, arg3 = 0}

      local _, err = reader.sendToDevice(emuMemclearCmd)
      if err then
         print('Failed to clear emulator memory:', err)
         return false
      else
         print('Cleared emulator memory')
      end
   end

   local CHUNK_SZ = 512
   local CHUNK_COUNT = CHUNK_SZ / 16

   -- Can fit 32 16 byte blocks per command (512 total bytes max)
   for i = 0, (data:len() / CHUNK_SZ) do
      local data_chunk = data:sub((i*CHUNK_SZ) + 1, (i*CHUNK_SZ) + CHUNK_SZ)
      print(string.format('Transmission #%u: %u bytes', i, data_chunk:len()))

      local emuMemsetCmd = Command:new{cmd = cmds.CMD_MIFARE_EML_MEMSET,
                                       data = hexlify(data_chunk),
                                       arg1 = i * CHUNK_COUNT,
                                       arg2 = CHUNK_COUNT}

      local _, err = reader.sendToDevice(emuMemsetCmd)
      if err then
         print('Failed setting memory:', err)
         return false
      end
   end

   print('Emulator memory set')
   return true
end


return Emulator
