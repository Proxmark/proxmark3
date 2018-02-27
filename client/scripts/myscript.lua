local cmds = require('commands')
local lib14a = require('read14a')


--- 
-- This is only meant to be used when errors occur
function oops(err)
	print("ERROR: ",err)
end

---
-- Used to send raw data to the firmware to subsequently forward the data to the card.
function sendRaw(rawdata, crc, power)
	print((">> 	  %s"):format(rawdata))

	local flags = lib14a.ISO14A_COMMAND.ISO14A_RAW
	if crc then
		flags = flags + lib14a.ISO14A_COMMAND.ISO14A_APPEND_CRC
	end
	if power then
		flags = flags + lib14a.ISO14A_COMMAND.ISO14A_NO_DISCONNECT
	end

	local command = Command:new{cmd = cmds.CMD_READER_ISO_14443a, 
								arg1 = flags, -- Send raw
								arg2 = string.len(rawdata) / 2, -- arg2 contains the length, which is half the length of the ASCII-string rawdata
								data = rawdata}
	local ignore_response = false
	local result, err = lib14a.sendToDevice(command, ignore_response)
	if result then
		--unpack the first 4 parts of the result as longs, and the last as an extremely long string to later be cut down based on arg1, the number of bytes returned
		local count,cmd,arg1,arg2,arg3,data = bin.unpack('LLLLH512',result)
		print(("<< %s"):format(string.sub(data, 1, arg1 * 2))) -- need to multiply by 2 because the hex digits are actually two bytes when they are strings
	else
		err = "Error sending the card raw data."
		oops(err)
	end
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
	GETVERS_INIT = "0360" -- Begins the GetVersion command
	GETVERS_CONT = "03AF" -- Continues the GetVersion command
	POWEROFF = "OFF"

	sendRaw(GETVERS_INIT, true, true)
	sendRaw(GETVERS_CONT, true, true)
	sendRaw(GETVERS_CONT, true, true)

	sendRaw(POWEROFF, false, false) --power off the Proxmark



end


main(args) -- Call the main function
