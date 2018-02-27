local cmds = require('commands')
local lib14a = require('read14a')


--- 
-- This is only meant to be used when errors occur
function oops(err)
	print("ERROR: ",err)
end

function sendRaw(rawdata, crc)
	print(">> ", rawdata)

	--get rid of the ISO14A_APPEND_CRC flag if we don't want a CRC to be appended to the raw bytes.
	local flags = lib14a.ISO14A_COMMAND.ISO14A_NO_DISCONNECT + lib14a.ISO14A_COMMAND.ISO14A_RAW + lib14a.ISO14A_COMMAND.ISO14A_APPEND_CRC

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

	-- Initialize the card using the read14a library
	info,err = lib14a.read14443a(true, no_rats)
	if err
		then oops(err)
		else print(("Connected to card with a UID of %s"):format(info.uid))
	end

	-- Now that the card is initialized, attempt to send raw data and read the response.
	getvers = "0360" -- 0x0360 begins the "get version" commands

	local result,err = sendRaw(getvers, true)
	if result then
		print("Currently trying to decode raw UsbCommand packet received.")
		local count,cmd,arg1,arg2,arg3,data = bin.unpack('LLLLH512',result)
		print(data)
	else
		err = "No response from sending the card raw data."
		oops(err)
	end

end


main(args) -- Call the main function
