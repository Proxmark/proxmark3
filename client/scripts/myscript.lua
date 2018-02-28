local cmds = require('commands')
local lib14a = require('read14a')

GETVERS_INIT = "0360" -- Begins the GetVersion command
GETVERS_CONT = "03AF" -- Continues the GetVersion command
POWEROFF = "OFF"
WRITEPERSO = "03A8"
COMMITPERSO = "03AA"
AUTH_FIRST = "0370"
AUTH_CONT = "0372"
AUTH_NONFIRST = "0376"

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
		returned_bytes = string.sub(data, 1, arg1 * 2)
		print(("<< %s"):format(returned_bytes)) -- need to multiply by 2 because the hex digits are actually two bytes when they are strings
		return returned_bytes
	else
		err = "Error sending the card raw data."
		oops(err)
	end
end

function writePerso()
	-- Used to write any data, including the keys (Key A and Key B), for all the sectors.
	-- writePerso() command parameters:
	--		1 byte  - 0xA8 - Command Code
	--		2 bytes - Address of the first block or key to be written to (40 blocks are numbered from 0x0000 to 0x00FF)
	--		X bytes - The data bytes to be written, starting from the first block. Amount of data sent can be from 16 to 240 bytes in 16 byte increments. This allows
	--				up to 15 blocks to be written at once.
	-- response from PICC:
	--		0x90 - OK
	--		0x09 - targeted block is invalid for writes, i.e. block 0, which contains manufacturer data
	--		0x0B - command invalid
	--		0x0C - unexpected command length
	
	SIXTEEN_BYTES_ZEROS = "00000000000000000000000000000000"

	-- First, set all the data in the card (4kB of data) to zeros. The keys, stored in the sector trailer block, are also set to zeros.
	-- The only block which cannot be explicitly set is block 0x0000, the manufacturer block.
	print("Setting values of normal blocks")
	for i=1,255,1 do --skip block 0
		--convert the number to hex with leading zeros, then use it as the block number in writeBlock()
		blocknum = string.format("%04x", i)
		writeBlock(blocknum, SIXTEEN_BYTES_ZEROS)
	end
	print("Finished setting values of normal blocks")

	print("Setting AES Sector keys")
	-- Next, write to the AES sector keys
	for i=0,39 do --for each sector number
		local keyA_block = "40" .. string.format("%02x", i * 2)
		local keyB_block = "40" .. string.format("%02x", (i * 2) + 1)
		--Can also calculate the keys fancily to make them unique, if desired
		keyA = SIXTEEN_BYTES_ZEROS
		keyB = SIXTEEN_BYTES_ZEROS
		writeBlock(keyA_block, keyA)
		writeBlock(keyB_block, keyB)
	end
	print("Finished setting AES Sector keys")

	print("Setting misc keys which haven't been set yet.")
	--CardMasterKey
	blocknum = "9000"
	writeBlock(blocknum, SIXTEEN_BYTES_ZEROS)
	--CardConfigurationKey
	blocknum = "9001"
	writeBlock(blocknum, SIXTEEN_BYTES_ZEROS)
	--L3SwitchKey
	blocknum = "9003"
	writeBlock(blocknum, SIXTEEN_BYTES_ZEROS)
	--SL1CardAuthKey
	blocknum = "9004"
	writeBlock(blocknum, SIXTEEN_BYTES_ZEROS)
	--L3SectorSwitchKey
	blocknum = "9006"
	writeBlock(blocknum, SIXTEEN_BYTES_ZEROS)
	--L1L3MixSectorSwitchKey
	blocknum = "9007"
	writeBlock(blocknum, SIXTEEN_BYTES_ZEROS)
	print("Finished setting misc keys.")

	print("WritePerso finished! Card is ready to move into new security level.")
end

function writeBlock(blocknum, data)
	-- Method writes 16 bytes of the string sent (data) to the specified block number
	-- The block numbers sent to the card need to be in little endian format (i.e. block 0x0001 is sent as 0x1000)
	blocknum_little_endian = string.sub(blocknum, 3, 4) .. string.sub(blocknum, 1, 2)
	commandString = WRITEPERSO .. blocknum_little_endian .. data --Write 16 bytes (32 hex chars).
	response = sendRaw(commandString, true, true) --0x90 is returned upon success
	if string.sub(response, 3, 4) ~= "90" then
		oops(("error occurred while trying to write to block %s"):format(blocknum))
	end
end

function authenticateAES()
	-- Used to try to authenticate with the AES keys we programmed into the card, to ensure the authentication works correctly.
	commandString = AUTH_FIRST
	commandString = commandString .. ""
end

function getVersion()
	sendRaw(GETVERS_INIT, true, true)
	sendRaw(GETVERS_CONT, true, true)
	sendRaw(GETVERS_CONT, true, true)
end

function commitPerso()
	commandString = COMMITPERSO .. "01" --switch to SL1
	response = sendRaw(commandString, true, true) --0x90 is returned upon success
	if string.sub(response, 3, 4) ~= "90" then
		oops("error occurred while trying to switch security level")
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

	-- Now, the card is initialized and we can do more interesting things.

	--writePerso()
	commitPerso()


	-- Power off the Proxmark
	sendRaw(POWEROFF, false, false)



end


main(args) -- Call the main function
