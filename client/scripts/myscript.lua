local cmds = require('commands')
local lib14a = require('read14a')


--- 
-- This is only meant to be used when errors occur
function oops(err)
	print("ERROR: ",err)
end

function sendRaw(rawdata, crc)
	print(">> ", rawdata)
	
	-- if crc
	-- 	then local flags = lib14a.ISO14A_COMMAND.ISO14A_NO_DISCONNECT + lib14a.ISO14A_COMMAND.ISO14A_RAW
	-- 	else local flags = lib14a.ISO14A_COMMAND.ISO14A_NO_DISCONNECT + lib14a.ISO14A_COMMAND.ISO14A_RAW + lib14a.ISO14A_APPEND_CRC
	-- end

	local flags = lib14a.ISO14A_COMMAND.ISO14A_NO_DISCONNECT + lib14a.ISO14A_COMMAND.ISO14A_RAW + lib14a.ISO14A_COMMAND.ISO14A_APPEND_CRC

	-- local flags = lib14a.ISO14A_COMMAND.ISO14A_NO_DISCONNECT + lib14a.ISO14A_COMMAND.ISO14A_RAW

	local command = Command:new{cmd = cmds.CMD_READER_ISO_14443a, 
								arg1 = flags, -- Send raw
								arg2 = string.len(rawdata) / 2, -- arg2 contains the length, which is half the length of the ASCII-string rawdata
								data = rawdata}
	local ignore_response = false
	return lib14a.sendToDevice(command, ignore_response)
end

--- 
-- The main entry point
function main(args)



	-- Manually send the card the init commands
	-- firstcommand = Command:new{cmd = cmds.CMD_READER_ISO_14443a, 
	-- 						  arg1 = lib14a.ISO14A_COMMAND.ISO14A_CONNECT + lib14a.ISO14A_COMMAND.ISO14A_NO_DISCONNECT,
	-- 						  arg2 = 0,
	-- 						  data = ""}
	-- local restwo,errtwo = lib14a.sendToDevice(firstcommand)
	-- print(firstcommand.arg1)

	-- Call the program via the command line
	-- result = core.console("hf 14a raw -p -b 7 -a 26") --I can do this from the command line, but I can't capture the output easily
	-- print(result)

	-- Send the card the init commands using the read14a library, reusing the connect functionality from a common library
	info,err = lib14a.read14443a(true, no_rats)
	if err
		then oops(err)
		else print(("Connected to card with a UID of %s"):format(info.uid))
	end

	--Attempt to send raw data
	getvers = "0360" -- 0x0360 should begin the "get version" commands
	--print(string.byte(getvers, 1, 99999))
	--crc = core.crc16(getvers) -- under the hood, calls ComputeCrc14443, which is the same function which is called by "hf 14a raw"
	--print(string.byte(crc, 1, 99999))

	local result,err = sendRaw(getvers, true)
	if result then
		print("Currently trying to decode raw UsbCommand packet received.")
		print(result)

		local count,cmd,arg1,arg2,arg3,data = bin.unpack('LLLLH512',result)
		print(data)
		--data = string.sub(result,count)
		--local cmd_response = Command.parse(res)
	else
		err ="No response from card"
	end
	-- local cmd_response = Command.parse(res)
	-- local len = tonumber(cmd_response.arg1) *2
	-- print("data length:",len)
	-- local data = string.sub(tostring(cmd_response.data), 0, len);
	-- print("<< ",data)

end

						

main(args) -- Call the main function
