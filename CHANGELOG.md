# Change Log
All notable changes to this project will be documented in this file.
This project uses the changelog in accordance with [keepchangelog](http://keepachangelog.com/). Please use this to write notable changes, which is not the same as git commit log...

## [unreleased][unreleased]

### Changed
- Changed hf mfp security. Now it works in all the modes. (drHatson)
- `hf fido` - show/check DER certificate and signatures (Merlok)
- Changed `lf hitag reader 0x ... <firstPage> <tagmode>` - to select first page to read and tagmode (0=STANDARD, 1=ADVANCED, 2=FAST_ADVANCED)
- Accept hitagS con0 tags with memory bits set to 11 and handle like 2048 tag

### Fixed
- AC-Mode decoding for HitagS
- Wrong UID at HitagS simulation
- `hf 15 sim` now works as expected (piwi)
- `hf mf chk t` save to emulator memory now works as expected (mwalker)
- Fix `hf mf sim` - wrong access rights to write key B in trailer (@McEloff)
- allow files > 512Bytes in 'hf iclass eload' (@Sherhannn79)
 
### Added
- Added to `hf 14a apdu` print apdu and compose apdu (@merlokk)
- Added `hf 15 csetuid` - set UID on ISO-15693 Magic tags (t0m4)
- Added `lf config s xxxx` option to allow skipping x samples before capture (marshmellow)
- Added `lf em 4x05protect` to support changing protection blocks on em4x05 chips (marshmellow)
- Support Standard Communication Mode in HITAG S
- Added `hf emv scan` - commands for scan EMV card and dump data to json file (Merlok)
- `hf mfp` group of commands (Merlok)
- Added `hf fido` - FIDO U2F authenticator commands https://fidoalliance.org/ (Merlok)
- Added `lf hitag reader 03` - read block (instead of pages) 
- Added `lf hitag reader 04` - read block (instead of pages) 
- Added `hf fido` `assert` and `make` commands from fido2 protocol (authenticatorMakeCredential and authenticatorGetAssertion) (Merlok)
- Added `lf paradox clone` to clone a Paradox card
- Added `emv` commands working for both contactless and smart cards (Merlok)
- Added `hf 15 snoop` (piwi)
- Added support for standard USB Smartcard Readers (piwi)
- Added `hf plot` (piwi)
- Added `hf mfp mad` `hf mf mad` parsing MAD1 and MAD2 (Merlok)
- Added `hf mfp ndef` `hf mf ndef` parsing NDEF records (Merlok)
- Added Mifare Mini, Mifare 2K and 4K support to `hf mf sim` (piwi)
- Added Legic detection to `hf search` (dnet)
- Added Home (Pos1) and End key bindings to the plot GUI (based on @mcd1992)
- Added downlink reference mode option r <mode> [ 0 - (or missing) default/fixed bit, 1 - long leading, 2 - leading 0 and 3 - 1 of 4 ] to `lf t55xx detect`, `lf t55xx read`, `lf t55xx write`, and `lf t55xx bruteforce`
- Added special option `r 4` to bruteforce, to try all downlink modes (0,1,2 and 3) for each password
- `hf mfu info` now checks the NXP Originality Signature if availabe (piwi)
- Added `hf mf personalize` to personalize the UID option of Mifare Classic EV1 cards (piwi)


## [v3.1.0][2018-10-10]

### Changed
- Adjusted `lf cmdread` to respond to client when complete and the client will then automatically call `data samples`
- Improved backdoor detection misbehaving magic s50/1k tag (Fl0-0)
- Deleted wipe functionality from `hf mf csetuid` (Merlok)
- Changed `hf mf nested` logic (Merlok)
- Added `hf mf nested` mode: autosearch keys for attack (from well known keys) (Merlok)
- `hf mf nested` Check keys after they have found (Merlok)
- `hf mf chk` Move main cycle to arm (Merlok)
- Changed proxmark command line parameter `flush` to `-f` or `-flush` (Merlok)
- Changed `hf 14a reader` to just request-anticolission-select sequence (Merlok)
- Changed `hf 14a raw` - works with LED's and some exchange logic (Merlok)
- Changed TLV parser messages to more convenient (Merlok)
- Rewritten Legic Prime reader (`hf legic reader`, `write` and `fill`) - it is using xcorrelation now (AntiCat)
- `hf 14a` commands works via argtable3 commandline parsing library (Merlok)
- HID LF operations on firmware updated for complete native support of long (>37 bit) HID tags (grauerfuchs)
- Changed Legic Prime tag simulator (`hf legic sim`) to run from 212 kHz SSP clock for better reliability (AntiCat)

### Fixed
- Changed start sequence in Qt mode (fix: short commands hangs main Qt thread) (Merlok)
- Changed driver file proxmark3.inf to support both old and new Product/Vendor IDs (piwi)
- Changed all command line parsers in `hf emv` commands to argtable (Merlok)
- Implemented AppNap API, fixing #283 and #627 OSX USB comm issues (AntiCat)

### Added
- Added `sc` smartcard (contact card) commands - reader, info, raw, upgrade, setclock, list (hardware version RDV4.0 only) must turn option on in makefile options (Willok, Iceman, marshmellow)
- Added a bitbang mode to `lf cmdread` if delay is 0 the cmd bits turn off and on the antenna with 0 and 1 respectively (marshmellow)
- Added PAC/Stanley detection to lf search (marshmellow)
- Added lf pac demod and lf pac read - extracts the raw blocks from a PAC/Stanley tag (marshmellow)
- Added hf mf c* commands compatibility for 4k and gen1b backdoor (Fl0-0)
- Added backdoor detection for gen1b magic s70/4k tag (Fl0-0)
- Added data fsktonrz, a fsk cleaning/demodulating routine for weak fsk signal. Note: follow this up with a `data rawdemod nr` to finish demoding your signal. (marshmellow)
- Added lf em 410xbrute, LF EM410x reader bruteforce attack by simulating UIDs from a file (Fl0-0)
- Added `hf mf cwipe` command. It wipes "magic Chinese" card. For 1a generation it uses card's "wipe" command. For gen1a and gen1b it uses a write command. (Merlok)
- Added to `hf mf nested` source key check before attack (Merlok)
- Added to `hf mf nested` after attack it checks all found keys on non-open sectors (Merlok)
- `hf mf chk` Added settings to set iso14443a operations timeout. default timeout set to 500us (Merlok)
- Added to `hf mf nested` parameters `s` and `ss` for checking slow cards (Merlok)
- Added to proxmark command line parameters `w` - wait 20s for serial port (Merlok)
- Added to proxmark command line parameters `c` and `l` - execute command and lua script from command line (Merlok)
- Added to proxmark ability to execute commands from stdin (pipe) (Merlok)
- Added `hf 14a info` and moved there functionality from `hf 14a reader` (Merlok)
- Added to `hf 14a info` detection of weak prng from Iceman1001 fork (Merlok)
- Added to `hf 14a apdu` - exchange apdu via iso1443-4 (Merlok)
- Added to `hf 14a apdu` - apdu and tlv results parser (Merlok)
- Added `hf emv` group of commands (Merlok)
- Added `hf emv search` `hf emv pse` - commands for selection of EMV application (Merlok)
- Added `hf emv select` - command for select EMV application (Merlok)
- Added `hf emv exec` - command for execute EMV transaction (Merlok)
- Added to `hf emv exec` MSD path for VISA and Mastercard and some other compatible EMV cards (Merlok)
- Added to `hf emv exec` SDA, DDA, fast DDA, CDA calculations for VISA and Mastercard and some other compatible EMV cards (Merlok)
- Added `hf emv test` - crypto tests for DES, AES, SHA, RSA, SDA, DDA, CDA and some other crypto functions (Merlok)
- Added `hf list mf` - deciphers crypto1 stream and works with first authentication and weak nested authentications (Merlok)
- Added to `hf emv` commands: `gpo`, `readrec`, `genac`, `challenge`, `intauth` - commands working with EMV cards (Merlok)
- Added `lf hid encode` and `lf hid decode` commands to translate printed HID card data to and from the packed data transmitted by a prox tag (grauerfuchs)
- Added `lf hid write` command, which operates as a macro for encode followed by clone operations (grauerfuchs)

## [3.0.1][2017-06-08]

### Fixed
- Compiles on OS X
- Compiles with gcc 4.9
- Compiles for non-Intel CPUs


## [3.0.0][2017-06-05]

### Added
- Added lf hitag write 24, the command writes a block to hitag2 tags in crypto mode (henjo)

### Added
- Added hf mf hardnested, an attack working for hardened Mifare cards (EV1, Mifare Plus SL1) where hf mf nested fails (piwi)
- Added experimental testmode write option for t55xx (danger) (marshmellow)
- Added t55xx p1detect to `lf search` chip detections (marshmellow)
- Added lf t55xx p1detect, detect page 1 of a t55xx tag based on E015 mfg code (marshmellow)
- Added lf noralsy demod, read, clone, sim commands (iceman)
- Added lf jablotron demod, read, clone, sim commands (iceman)
- Added lf nexwatch read   - reads a nexwatch tag from the antenna
- Added lf paradox read    - reads a paradox tag from the antenna
- Added lf fdx sim (iceman)
- Added lf fdx clone       - clones an fdx-b animal tag to t55x7 or q5 (iceman)
- Added lf fdx read        - reads a fdx-b tag from the antenna (iceman)
- Added lf gproxii read    - reads a gproxii tag from the antenna (marshmellow)
- Added lf indala read     - reads an indala tag from the antenna (marshmellow)
- Added lf visa2000 demod, read, clone, sim commands (iceman)
- Added markers in the graph around found Sequence Terminator after askmandemod.
- Added data mtrim <start> <stop> command to trim out samples between start and stop
- Added data setgraphmarkers <orange> <blue> command to set two extra markers on the graph (marshmellow)
- Added EM4x05/EM4x69 chip detection to lf search (marshmellow) 
- Added lf em 4x05dump command to read and output all the blocks of the chip (marshmellow)
- Added lf em 4x05info command to read and display information about the chip (marshmellow)
- Added lf cotag read, and added it to lf search (iceman)
- Added hitag2 read UID only and added that to lf search (marshmellow)
- Added lf pyramid commands (iceman)
- Added lf presco commands - some bits not fully understood... (iceman)
- Added experimental HitagS support (Oguzhan Cicek, Hendrik Schwartke, Ralf Spenneberg)
  see https://media.ccc.de/v/32c3-7166-sicherheit_von_125khz_transpondern_am_beispiel_hitag_s
  English video available
- Added a LF ASK Sequence Terminator detection option to the standard ask demod - and applied it to `lf search u`, `lf t55xx detect`, and `data rawdemod am s` (marshmellow)
- `lf t55xx bruteforce <start password> <end password> [i <*.dic>]` - Simple bruteforce attack to find password - (iceman and others)
- `lf viking clone`- clone viking tag to t55x7 or Q5 from 4byte hex ID input 
- `lf viking sim`  - sim full viking tag from 4byte hex ID input
- `lf viking read` - read viking tag and output ID
- `lf t55xx wipe`  - sets t55xx back to factory defaults
- Added viking demod to `lf search` (marshmellow)
- `lf viking demod` demod viking id tag from graphbuffer (marshmellow)
- `lf t55xx resetread` added reset then read command - should allow determining start of stream transmissions (marshmellow)
- `lf t55xx wakeup` added wake with password (AOR) to allow lf search or standard lf read after (iceman, marshmellow)
- `hf iclass managekeys` to save, load and manage iclass keys.  (adjusted most commands to accept a loaded key in memory) (marshmellow)
- `hf iclass readblk` to select, authenticate, and read 1 block from an iclass card (marshmellow)
- `hf iclass writeblk` to select, authenticate, and write 1 block to an iclass card (or picopass) (marshmellow + others)
- `hf iclass clone` to take a saved dump file and clone selected blocks to a new tag (marshmellow + others)
- `hf iclass calcnewkey` - to calculate the div_key change to change a key - (experimental) (marshmellow + others)
- `hf iclass encryptblk` - to encrypt a data block hex to prep for writing that block (marshmellow)
- ISO14443a stand-alone operation with ARM CFLAG="WITH_ISO14443a_StandAlone". This code can read & emulate two banks of 14a tag UIDs and write to "magic" cards  (Craig Young) 
- AWID26 command context added as 'lf awid' containing realtime demodulation as well as cloning/simulation based on tag numbers (Craig Young)
- Added 'hw status'. This command makes the ARM print out some runtime information. (holiman) 
- Added 'hw ping'. This command just sends a usb packets and checks if the pm3 is responsive. Can be used to abort certain operations which supports abort over usb. (holiman)
- Added `data hex2bin` and `data bin2hex` for command line conversion between binary and hexadecimal (holiman)
- Added 'hf snoop'. This command take digitalized signal from FPGA and put in BigBuffer. (pwpiwi + enio)
- Added Topaz (NFC type 1) protocol support ('hf topaz reader', 'hf list topaz', 'hf 14a raw -T', 'hf topaz snoop'). (piwi)
- Added option c to 'hf list' (mark CRC bytes) (piwi)

### Changed
- Adjusted the lf demods to auto align and set the grid for the graph plot. 
- `lf snoop` now automatically gets samples from the device
- `lf read` now accepts [#samples] as arg. && now automatically gets samples from the device
- adjusted lf t5 chip timings to use WaitUS. and adjusted the readblock timings
    appears to have more consistent results with more antennas.
- `lf t5 wakeup` has been adjusted to not need the p in front of the pwd arg.
- `data psknexwatchdemod` has been moved to `lf nexwatch demod` (reads from graphbuffer)
- `data fskparadoxdemod` has been moved to `lf paradox demod` (reads from graphbuffer)
- `data fdxdemod` has been moved to `lf fdx demod` (reads from graphbuffer)
- `data askgproxiidemod has been moved to `lf gproxii demod` (reads from graphbuffer)
- `lf indalaclone` has been moved to `lf indala clone`
- `lf indalademod` has been moved to `lf indala altdemod` (reads from graphbuffer)
- `data pskindalademod` has been moved to `lf indala demod` (reads from graphbuffer)
- `data askvikingdemod` has been moved to `lf viking demod` (reads from graphbuffer)
- `data fskpyramiddemod` has been moved to `lf pyramid demod` (reads from graphbuffer)
- `data fskiodemod` has been moved to `lf io demod` (reads from graphbuffer)
- `lf io fskdemod` has been renamed to `lf io read` (reads from antenna)
- `data fskawiddemod` has been moved to `lf awid demod` (reads from graphbuffer)
- `lf awid fskdemod` has been renamed to `lf awid read` (reads from antenna)
- `data fskhiddemod` has been moved to `lf hid demod` (reads from graphbuffer)
- `lf hid demod` has been renamed to `lf hid read` (reads from antenna)
- all em410x demod and print functions moved to cmdlfem4x.c
- `data askem410xdemod` has been moved to `lf em 410xdemod` (reads from graphbuffer)
- `lf em 410xdemod` has been renamed to `lf em 410xread` (reads from antenna)
- hf mf dump - added retry loops to try each read attempt up to 3 times.  makes getting a complete dump easier with many antennas. 
- small changes to lf psk and fsk demods to improve results when the trace begins with noise or the chip isn't broadcasting yet (marshmellow)
- NOTE CHANGED ALL `lf em4x em*` cmds to simpler `lf em ` - example: `lf em4x em410xdemod` is now `lf em 410xdemod`
- Renamed and rebuilt `lf em readword` && readwordpwd to `lf em 4x05readword` - it now demods and outputs the read block (marshmellow/iceman)
- Renamed and rebuilt `lf em writeword` && writewordpwd to `lf em 4x05writeword` - it now also reads validation output from the tag (marshmellow/iceman)
- Fixed bug in lf sim and continuous demods not turning off antenna when finished
- Fixed bug(s) in hf iclass write
- Fixed bug in lf biphase sim - `lf simask b` (and any tagtype that relies on it - gproxii...) (marshmellow)
- Fixed bug in lf viking clone/sim (iceman)
- Fixed broken `data askedgedetect` (marshmellow)
- Adjusted hf mf sim command (marshmellow)
    added auto run mfkey32 to extract all keys 
    also added f parameter to allow attacking with UIDs from a file (implies x and i parameters)
    also added e parameter to allow adding the extracted keys to emulator memory for the next simulation
    added 10 byte uid option
- Added `[l] <length>` option to data printdemodbuffer (marshmellow)
- Adjusted lf awid clone to optionally clone to Q5 tags (marshmellow)
- Adjusted lf t55xx detect to find Q5 tags (t5555) instead of just t55x7 (marshmellow)
- Adjusted all lf NRZ demods - works more accurately and consistently (as long as you have strong signal) (marshmellow)
- Adjusted lf pskindalademod to reduce false positive reads. (marshmellow)
- Small adjustments to psk, nrz, and ask clock detect routines - more reliable. (marshmellow)
- Adjusted lf em410x em410xsim to accept a clock argument (marshmellow)
- Adjusted lf t55xx dump to allow overriding the safety check and warning text (marshmellow)
- Adjusted lf t55xx write input variables (marshmellow)
- Adjusted lf t55xx read with password safety check and warning text and adjusted the input variables (marshmellow & iceman)
- Adjusted LF FSK demod to account for cross threshold fluctuations (898 count waves will adjust the 9 to 8 now...) more accurate. (marshmellow)
- Adjusted timings for t55xx commands.  more reliable now. (marshmellow & iceman)
- `lf cmdread` adjusted input methods and added help text (marshmellow & iceman)
- changed `lf config t <threshold>` to be 0 - 128 and will trigger on + or - threshold value (marshmellow) 
- `hf iclass dump` cli options - can now dump AA1 and AA2 with different keys in one run (does not go to multiple pages for the larger tags yet) (marshmellow)
- Revised workflow for StandAloneMode14a (Craig Young)
- EPA functions (`hf epa`) now support both ISO 14443-A and 14443-B cards (frederikmoellers)
- 'hw version' only talks to ARM at startup, after that the info is cached. (pwpiwi)
- Added `r` option to iclass functions - allows key to be provided in raw block 3/4 format 

## [2.2.0][2015-07-12]

### Changed
- Added `hf 14b raw -s` option to auto select a 14b std tag before raw command 
- Changed `hf 14b write` to `hf 14b sriwrite` as it only applied to sri tags (marshmellow)
- Added `hf 14b info` to `hf search` (marshmellow)
- Added compression of fpga config and data, *BOOTROM REFLASH REQUIRED* (piwi)
- Implemented better detection of mifare-tags that are not vulnerable to classic attacks (`hf mf mifare`, `hf mf nested`) (piwi)

### Added
- Add `hf 14b info` to find and print info about std 14b tags and sri tags (using 14b raw commands in the client)  (marshmellow)
- Add PACE replay functionality (frederikmoellers)

### Fixed 
- t55xx write timing (marshmellow)


## [2.1.0][2015-06-23]

### Changed
- Added ultralight/ntag tag type detection to `hf 14a read` (marshmellow)
- Improved ultralight dump command to auto detect tag type, take authentication, and dump full memory (or subset specified) of known tag types (iceman1001 / marshmellow)
- Combined ultralight read/write commands and added authentication (iceman1001)
- Improved LF manchester and biphase demodulation and ask clock detection especially for reads with heavy clipping. (marshmellow)
- Iclass read, `hf iclass read` now also reads tag config and prints configuration. (holiman)
- *bootrom* needs to be flashed, due to new address boundaries between os and fpga, after a size optimization (piwi)

### Fixed
- Fixed EM4x50 read/demod of the tags broadcasted memory blocks. 'lf em4x em4x50read' (not page read) (marshmellow)
- Fixed issue #19, problems with LF T55xx commands (iceman1001, marshmellow)
- Fixed various problems with iso14443b, issue #103 (piwi, marshmellow)

### Added
- Added `hf search` - currently tests for 14443a tags, iclass tags, and 15693 tags (marshmellow) 
- Added `hf mfu info` Ultralight/NTAG info command - reads tag configuration and info, allows authentication if needed (iceman1001, marshmellow)
- Added Mifare Ultralight C and Ultralight EV1/NTAG authentication. (iceman1001)
- Added changelog

## [2.0.0] - 2015-03-25
### Changed
- LF sim operations now abort when new commands arrive over the USB - not required to push the device button anymore.

### Fixed
- Mifare simulation, `hf mf sim` (was broken a long time) (pwpiwi)
- Major improvements in LF area and data operations. (marshmellow, iceman1001)
- Issues regarding LF simulation (pwpiwi)

### Added
- iClass functionality: full simulation of iclass tags, so tags can be simulated with data (not only CSN). Not yet support for write/update, but readers don't seem to enforce update. (holiman).
- iClass decryption. Proxmark can now decrypt data on an iclass tag, but requires you to have the HID decryption key locally on your computer, as this is not bundled with the sourcecode. 
