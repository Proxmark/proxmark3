To test the nonce2key tool.

:: tip
You can use the output from "hf mf mifare"  to use with this tool. 

:: sample
./nonce2key 1b2a28f5 73a4c24c 00000000 734b3b93eb4bd303 0d0d060f0f0f0200

If all parity bits are 0, it is assumed that this is a Mifare clone responding with NACK to all wrong authentication attempts.
In this case, a special attack is used (which usually results in many possible keys)

:: sample with all parity bits 0:
./nonce2key 2e086b1a 2210af4e 00000002 0000000000000000 050708040a030b06
