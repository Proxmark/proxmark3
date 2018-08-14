//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency EM4x commands
//-----------------------------------------------------------------------------

#include "cmdlfem4x.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "comms.h"
#include "ui.h"
#include "util.h"
#include "graph.h"
#include "cmdparser.h"
#include "cmddata.h"
#include "cmdlf.h"
#include "cmdmain.h"
#include "lfdemod.h"
#include "protocols.h"
#include "util_posix.h"

uint64_t g_em410xId=0;

static int CmdHelp(const char *Cmd);
void ConstructEM410xEmulGraph(const char *uid,const  uint8_t clock);

int CmdEMdemodASK(const char *Cmd)
{
	char cmdp = param_getchar(Cmd, 0);
	int findone = (cmdp == '1') ? 1 : 0;
	UsbCommand c={CMD_EM410X_DEMOD};
	c.arg[0]=findone;
	SendCommand(&c);
	return 0;
}

//by marshmellow
//print 64 bit EM410x ID in multiple formats
void printEM410x(uint32_t hi, uint64_t id)
{
	if (id || hi){
		uint64_t iii=1;
		uint64_t id2lo=0;
		uint32_t ii=0;
		uint32_t i=0;
		for (ii=5; ii>0;ii--){
			for (i=0;i<8;i++){
				id2lo=(id2lo<<1LL) | ((id & (iii << (i+((ii-1)*8)))) >> (i+((ii-1)*8)));
			}
		}
		if (hi){
			//output 88 bit em id
			PrintAndLog("\nEM TAG ID      : %06X%016" PRIX64, hi, id);
		} else{
			//output 40 bit em id
			PrintAndLog("\nEM TAG ID      : %010" PRIX64, id);
			PrintAndLog("\nPossible de-scramble patterns");
			PrintAndLog("Unique TAG ID  : %010" PRIX64,  id2lo);
			PrintAndLog("HoneyWell IdentKey {");
			PrintAndLog("DEZ 8          : %08" PRIu64,id & 0xFFFFFF);
			PrintAndLog("DEZ 10         : %010" PRIu64,id & 0xFFFFFFFF);
			PrintAndLog("DEZ 5.5        : %05lld.%05" PRIu64,(id>>16LL) & 0xFFFF,(id & 0xFFFF));
			PrintAndLog("DEZ 3.5A       : %03lld.%05" PRIu64,(id>>32ll),(id & 0xFFFF));
			PrintAndLog("DEZ 3.5B       : %03lld.%05" PRIu64,(id & 0xFF000000) >> 24,(id & 0xFFFF));
			PrintAndLog("DEZ 3.5C       : %03lld.%05" PRIu64,(id & 0xFF0000) >> 16,(id & 0xFFFF));
			PrintAndLog("DEZ 14/IK2     : %014" PRIu64,id);
			PrintAndLog("DEZ 15/IK3     : %015" PRIu64,id2lo);
			PrintAndLog("DEZ 20/ZK      : %02" PRIu64 "%02" PRIu64 "%02" PRIu64 "%02" PRIu64 "%02" PRIu64 "%02" PRIu64 "%02" PRIu64 "%02" PRIu64 "%02" PRIu64 "%02" PRIu64,
			    (id2lo & 0xf000000000) >> 36,
			    (id2lo & 0x0f00000000) >> 32,
			    (id2lo & 0x00f0000000) >> 28,
			    (id2lo & 0x000f000000) >> 24,
			    (id2lo & 0x0000f00000) >> 20,
			    (id2lo & 0x00000f0000) >> 16,
			    (id2lo & 0x000000f000) >> 12,
			    (id2lo & 0x0000000f00) >> 8,
			    (id2lo & 0x00000000f0) >> 4,
			    (id2lo & 0x000000000f)
			);
			uint64_t paxton = (((id>>32) << 24) | (id & 0xffffff))  + 0x143e00;
			PrintAndLog("}\nOther          : %05" PRIu64 "_%03" PRIu64 "_%08" PRIu64 "",(id&0xFFFF),((id>>16LL) & 0xFF),(id & 0xFFFFFF));
			PrintAndLog("Pattern Paxton : %" PRIu64 " [0x%" PRIX64 "]", paxton, paxton);

			uint32_t p1id = (id & 0xFFFFFF);
			uint8_t arr[32] = {0x00};
			int i =0;
			int j = 23;
			for (; i < 24; ++i, --j	){
				arr[i] = (p1id >> i) & 1;
			}

			uint32_t p1  = 0;

			p1 |= arr[23] << 21;
			p1 |= arr[22] << 23;
			p1 |= arr[21] << 20;
			p1 |= arr[20] << 22;

			p1 |= arr[19] << 18;
			p1 |= arr[18] << 16;
			p1 |= arr[17] << 19;
			p1 |= arr[16] << 17;

			p1 |= arr[15] << 13;
			p1 |= arr[14] << 15;
			p1 |= arr[13] << 12;
			p1 |= arr[12] << 14;

			p1 |= arr[11] << 6;
			p1 |= arr[10] << 2;
			p1 |= arr[9]  << 7;
			p1 |= arr[8]  << 1;

			p1 |= arr[7]  << 0;
			p1 |= arr[6]  << 8;
			p1 |= arr[5]  << 11;
			p1 |= arr[4]  << 3;

			p1 |= arr[3]  << 10;
			p1 |= arr[2]  << 4;
			p1 |= arr[1]  << 5;
			p1 |= arr[0]  << 9;
			PrintAndLog("Pattern 1      : %d [0x%X]", p1, p1);

			uint16_t sebury1 = id & 0xFFFF;
			uint8_t  sebury2 = (id >> 16) & 0x7F;
			uint32_t sebury3 = id & 0x7FFFFF;
			PrintAndLog("Pattern Sebury : %d %d %d  [0x%X 0x%X 0x%X]", sebury1, sebury2, sebury3, sebury1, sebury2, sebury3);
		}
	}
	return;
}

/* Read the ID of an EM410x tag.
 * Format:
 *   1111 1111 1           <-- standard non-repeatable header
 *   XXXX [row parity bit] <-- 10 rows of 5 bits for our 40 bit tag ID
 *   ....
 *   CCCC                  <-- each bit here is parity for the 10 bits above in corresponding column
 *   0                     <-- stop bit, end of tag
 */
int AskEm410xDecode(bool verbose, uint32_t *hi, uint64_t *lo )
{
	size_t idx = 0;
	uint8_t BitStream[512]={0};
	size_t BitLen = sizeof(BitStream);
	if ( !getDemodBuf(BitStream, &BitLen) ) return 0;

	if (Em410xDecode(BitStream, &BitLen, &idx, hi, lo)) {
		//set GraphBuffer for clone or sim command
		setDemodBuf(DemodBuffer, (BitLen==40) ? 64 : 128, idx+1);
		setClockGrid(g_DemodClock, g_DemodStartIdx + ((idx+1)*g_DemodClock));

		if (g_debugMode) {
			PrintAndLog("DEBUG: idx: %d, Len: %d, Printing Demod Buffer:", idx, BitLen);
			printDemodBuff();
		}
		if (verbose) {
			PrintAndLog("EM410x pattern found: ");
			printEM410x(*hi, *lo);
			g_em410xId = *lo;
		}
		return 1;
	}
	return 0;
}

//askdemod then call Em410xdecode
int AskEm410xDemod(const char *Cmd, uint32_t *hi, uint64_t *lo, bool verbose)
{
	bool st = true;
	if (!ASKDemod_ext(Cmd, false, false, 1, &st)) return 0;
	return AskEm410xDecode(verbose, hi, lo);
}

//by marshmellow
//takes 3 arguments - clock, invert and maxErr as integers
//attempts to demodulate ask while decoding manchester
//prints binary found and saves in graphbuffer for further commands
int CmdAskEM410xDemod(const char *Cmd)
{
	char cmdp = param_getchar(Cmd, 0);
	if (strlen(Cmd) > 10 || cmdp == 'h' || cmdp == 'H') {
		PrintAndLog("Usage:  lf em 410xdemod [clock] <0|1> [maxError]");
		PrintAndLog("     [set clock as integer] optional, if not set, autodetect.");
		PrintAndLog("     <invert>, 1 for invert output");
		PrintAndLog("     [set maximum allowed errors], default = 100.");
		PrintAndLog("");
		PrintAndLog("    sample: lf em 410xdemod        = demod an EM410x Tag ID from GraphBuffer");
		PrintAndLog("          : lf em 410xdemod 32     = demod an EM410x Tag ID from GraphBuffer using a clock of RF/32");
		PrintAndLog("          : lf em 410xdemod 32 1   = demod an EM410x Tag ID from GraphBuffer using a clock of RF/32 and inverting data");
		PrintAndLog("          : lf em 410xdemod 1      = demod an EM410x Tag ID from GraphBuffer while inverting data");
		PrintAndLog("          : lf em 410xdemod 64 1 0 = demod an EM410x Tag ID from GraphBuffer using a clock of RF/64 and inverting data and allowing 0 demod errors");
		return 0;
	}
	uint64_t lo = 0;
	uint32_t hi = 0;
	return AskEm410xDemod(Cmd, &hi, &lo, true);
}

int usage_lf_em410x_sim(void) {
	PrintAndLog("Simulating EM410x tag");
	PrintAndLog("");
	PrintAndLog("Usage:  lf em 410xsim [h] <uid> <clock>");
	PrintAndLog("Options:");
	PrintAndLog("       h         - this help");
	PrintAndLog("       uid       - uid (10 HEX symbols)");
	PrintAndLog("       clock     - clock (32|64) (optional)");
	PrintAndLog("samples:");
	PrintAndLog("      lf em 410xsim 0F0368568B");
	PrintAndLog("      lf em 410xsim 0F0368568B 32");
	return 0;
}

// Construct the graph for emulating an EM410X tag
void ConstructEM410xEmulGraph(const char *uid,const  uint8_t clock)
{
	int i, n, j, binary[4], parity[4];
	/* clear our graph */
	ClearGraph(0);

	/* write 9 start bits */
	for (i = 0; i < 9; i++)
		AppendGraph(0, clock, 1);

	/* for each hex char */
	parity[0] = parity[1] = parity[2] = parity[3] = 0;
	for (i = 0; i < 10; i++){
		/* read each hex char */
		sscanf(&uid[i], "%1x", &n);
		for (j = 3; j >= 0; j--, n/= 2)
			binary[j] = n % 2;

		/* append each bit */
		AppendGraph(0, clock, binary[0]);
		AppendGraph(0, clock, binary[1]);
		AppendGraph(0, clock, binary[2]);
		AppendGraph(0, clock, binary[3]);

		/* append parity bit */
		AppendGraph(0, clock, binary[0] ^ binary[1] ^ binary[2] ^ binary[3]);

		/* keep track of column parity */
		parity[0] ^= binary[0];
		parity[1] ^= binary[1];
		parity[2] ^= binary[2];
		parity[3] ^= binary[3];
	}

	/* parity columns */
	AppendGraph(0, clock, parity[0]);
	AppendGraph(0, clock, parity[1]);
	AppendGraph(0, clock, parity[2]);
	AppendGraph(0, clock, parity[3]);

	/* stop bit */
	AppendGraph(1, clock, 0);
}

// emulate an EM410X tag
int CmdEM410xSim(const char *Cmd)
{
	char cmdp = param_getchar(Cmd, 0);
	uint8_t uid[5] = {0x00};

	if (cmdp == 'h' || cmdp == 'H') return usage_lf_em410x_sim();
	/* clock is 64 in EM410x tags */
	uint8_t clock = 64;

	if (param_gethex(Cmd, 0, uid, 10)) {
		PrintAndLog("UID must include 10 HEX symbols");
		return 0;
	}
	param_getdec(Cmd,1, &clock);

	PrintAndLog("Starting simulating UID %02X%02X%02X%02X%02X  clock: %d", uid[0],uid[1],uid[2],uid[3],uid[4],clock);
	PrintAndLog("Press pm3-button to abort simulation");

	ConstructEM410xEmulGraph(Cmd, clock);
	
	CmdLFSim("0"); //240 start_gap.
	return 0;
}

int usage_lf_em410x_brute(void) {
	PrintAndLog("Bruteforcing by emulating EM410x tag");
	PrintAndLog("");
	PrintAndLog("Usage:  lf em 410xbrute [h] ids.txt [d 2000] [c clock]");
	PrintAndLog("Options:");
	PrintAndLog("       h             - this help");
	PrintAndLog("       ids.txt       - file with UIDs in HEX format, one per line");
	PrintAndLog("       d (2000)      - pause delay in milliseconds between UIDs simulation, default 1000 ms (optional)");
	PrintAndLog("       c (32)        - clock (32|64), default 64 (optional)");
	PrintAndLog("samples:");
	PrintAndLog("      lf em 410xbrute ids.txt");
	PrintAndLog("      lf em 410xbrute ids.txt c 32");
	PrintAndLog("      lf em 410xbrute ids.txt d 3000");
	PrintAndLog("      lf em 410xbrute ids.txt d 3000 c 32");
	return 0;
}

int CmdEM410xBrute(const char *Cmd)
{
	char filename[FILE_PATH_SIZE]={0};
	FILE *f = NULL;
	char buf[11];
	uint32_t uidcnt = 0;
	uint8_t stUidBlock = 20;
	uint8_t *uidBlock = NULL, *p = NULL;
	int ch;
	uint8_t uid[5] = {0x00};
	/* clock is 64 in EM410x tags */
	uint8_t clock = 64;
	/* default pause time: 1 second */
	uint32_t delay = 1000;
	
	char cmdp = param_getchar(Cmd, 0);
	
	if (cmdp == 'h' || cmdp == 'H') return usage_lf_em410x_brute();
	

	cmdp = param_getchar(Cmd, 1);
	
	if (cmdp == 'd' || cmdp == 'D') {
		delay = param_get32ex(Cmd, 2, 1000, 10);
		param_getdec(Cmd, 4, &clock);
	} else if (cmdp == 'c' || cmdp == 'C') {
		param_getdec(Cmd, 2, &clock);
		delay = param_get32ex(Cmd, 4, 1000, 10);
	}

	param_getstr(Cmd, 0, filename, sizeof(filename));
	
	uidBlock = calloc(stUidBlock, 5);
	if (uidBlock == NULL) return 1;

	if (strlen(filename) > 0) {
		if ((f = fopen(filename, "r")) == NULL) {
			PrintAndLog("Error: Could not open UIDs file [%s]",filename);
			free(uidBlock);
			return 1;
		}
	} else {
		PrintAndLog("Error: Please specify a filename");
		free(uidBlock);
		return 1;
	}

	while( fgets(buf, sizeof(buf), f) ) {
		if (strlen(buf) < 10 || buf[9] == '\n') continue;
		while (fgetc(f) != '\n' && !feof(f));  //goto next line

		//The line start with # is comment, skip
		if( buf[0]=='#' ) continue;
		
		if (param_gethex(buf, 0, uid, 10)) {
			PrintAndLog("UIDs must include 10 HEX symbols");
			free(uidBlock);
			fclose(f);
			return 1;
		}
		
		buf[10] = 0;
		
		if ( stUidBlock - uidcnt < 2) {
				p = realloc(uidBlock, 5*(stUidBlock+=10));
				if (!p) {
					PrintAndLog("Cannot allocate memory for UIDs");
					free(uidBlock);
					fclose(f);
					return 1;
				}
				uidBlock = p;
		}
		memset(uidBlock + 5 * uidcnt, 0, 5);
		num_to_bytes(strtoll(buf, NULL, 16), 5, uidBlock + 5*uidcnt);
		uidcnt++;
	  	memset(buf, 0, sizeof(buf));
	}
	fclose(f);
	
	if (uidcnt == 0) {
		PrintAndLog("No UIDs found in file");
		free(uidBlock);
		return 1;
	}
	PrintAndLog("Loaded %d UIDs from %s, pause delay: %d ms", uidcnt, filename, delay);
	
	// loop
	for(uint32_t c = 0; c < uidcnt; ++c ) {
		char testuid[11];
		testuid[10] = 0;
		
		if (ukbhit()) {
			ch = getchar();
			(void)ch;
			printf("\nAborted via keyboard!\n");
			free(uidBlock);
			return 0;
		}
				
		sprintf(testuid, "%010" PRIX64, bytes_to_num(uidBlock + 5*c, 5));
		PrintAndLog("Bruteforce %d / %d: simulating UID  %s, clock %d", c + 1, uidcnt, testuid, clock);
		
		ConstructEM410xEmulGraph(testuid, clock);
		
		CmdLFSim("0"); //240 start_gap.

		msleep(delay);
	}
	
	free(uidBlock);
	return 0;
}


/* Function is equivalent of lf read + data samples + em410xread
 * looped until an EM410x tag is detected
 *
 * Why is CmdSamples("16000")?
 *  TBD: Auto-grow sample size based on detected sample rate.  IE: If the
 *       rate gets lower, then grow the number of samples
 *  Changed by martin, 4000 x 4 = 16000,
 *  see http://www.proxmark.org/forum/viewtopic.php?pid=7235#p7235
 *
 *  EDIT -- capture enough to get 2 complete preambles at the slowest data rate known to be used (rf/64) (64*64*2+9 = 8201)	marshmellow
*/
int CmdEM410xWatch(const char *Cmd)
{
	do {
		if (ukbhit()) {
			printf("\naborted via keyboard!\n");
			break;
		}
		lf_read(true, 8201);
	} while (!CmdAskEM410xDemod(""));

	return 0;
}

//currently only supports manchester modulations
int CmdEM410xWatchnSpoof(const char *Cmd)
{
	CmdEM410xWatch(Cmd);
	PrintAndLog("# Replaying captured ID: %010"PRIx64, g_em410xId);
	CmdLFaskSim("");
	return 0;
}

int CmdEM410xWrite(const char *Cmd)
{
	uint64_t id = 0xFFFFFFFFFFFFFFFF; // invalid id value
	int card = 0xFF; // invalid card value
	unsigned int clock = 0; // invalid clock value

	sscanf(Cmd, "%" SCNx64 " %d %d", &id, &card, &clock);

	// Check ID
	if (id == 0xFFFFFFFFFFFFFFFF) {
		PrintAndLog("Error! ID is required.\n");
		return 0;
	}
	if (id >= 0x10000000000) {
		PrintAndLog("Error! Given EM410x ID is longer than 40 bits.\n");
		return 0;
	}

	// Check Card
	if (card == 0xFF) {
		PrintAndLog("Error! Card type required.\n");
		return 0;
	}
	if (card < 0) {
		PrintAndLog("Error! Bad card type selected.\n");
		return 0;
	}

	// Check Clock
	// Default: 64
	if (clock == 0)
		clock = 64;

	// Allowed clock rates: 16, 32, 40 and 64
	if ((clock != 16) && (clock != 32) && (clock != 64) && (clock != 40)) {
		PrintAndLog("Error! Clock rate %d not valid. Supported clock rates are 16, 32, 40 and 64.\n", clock);
		return 0;
	}

	if (card == 1) {
		PrintAndLog("Writing %s tag with UID 0x%010" PRIx64 " (clock rate: %d)", "T55x7", id, clock);
		// NOTE: We really should pass the clock in as a separate argument, but to
		//   provide for backwards-compatibility for older firmware, and to avoid
		//   having to add another argument to CMD_EM410X_WRITE_TAG, we just store
		//   the clock rate in bits 8-15 of the card value
		card = (card & 0xFF) | ((clock << 8) & 0xFF00);
	}	else if (card == 0) {
		PrintAndLog("Writing %s tag with UID 0x%010" PRIx64, "T5555", id, clock);
		card = (card & 0xFF) | ((clock << 8) & 0xFF00);
	} else {
		PrintAndLog("Error! Bad card type selected.\n");
		return 0;
	}

	UsbCommand c = {CMD_EM410X_WRITE_TAG, {card, (uint32_t)(id >> 32), (uint32_t)id}};
	SendCommand(&c);

	return 0;
}

//**************** Start of EM4x50 Code ************************
bool EM_EndParityTest(uint8_t *BitStream, size_t size, uint8_t rows, uint8_t cols, uint8_t pType)
{
	if (rows*cols>size) return false;
	uint8_t colP=0;
	//assume last col is a parity and do not test
	for (uint8_t colNum = 0; colNum < cols-1; colNum++) {
		for (uint8_t rowNum = 0; rowNum < rows; rowNum++) {
			colP ^= BitStream[(rowNum*cols)+colNum];
		}
		if (colP != pType) return false;
	}
	return true;
}

bool EM_ByteParityTest(uint8_t *BitStream, size_t size, uint8_t rows, uint8_t cols, uint8_t pType)
{
	if (rows*cols>size) return false;
	uint8_t rowP=0;
	//assume last row is a parity row and do not test
	for (uint8_t rowNum = 0; rowNum < rows-1; rowNum++) {
		for (uint8_t colNum = 0; colNum < cols; colNum++) {
			rowP ^= BitStream[(rowNum*cols)+colNum];
		}
		if (rowP != pType) return false;
	}
	return true;
}

uint32_t OutputEM4x50_Block(uint8_t *BitStream, size_t size, bool verbose, bool pTest)
{
	if (size<45) return 0;
	uint32_t code = bytebits_to_byte(BitStream,8);
	code = code<<8 | bytebits_to_byte(BitStream+9,8);
	code = code<<8 | bytebits_to_byte(BitStream+18,8);
	code = code<<8 | bytebits_to_byte(BitStream+27,8);
	if (verbose || g_debugMode){
		for (uint8_t i = 0; i<5; i++){
			if (i == 4) PrintAndLog(""); //parity byte spacer
			PrintAndLog("%d%d%d%d%d%d%d%d %d -> 0x%02x",
			    BitStream[i*9],
			    BitStream[i*9+1],
			    BitStream[i*9+2],
			    BitStream[i*9+3],
			    BitStream[i*9+4],
			    BitStream[i*9+5],
			    BitStream[i*9+6],
			    BitStream[i*9+7],
			    BitStream[i*9+8],
			    bytebits_to_byte(BitStream+i*9,8)
			);
		}
		if (pTest)
			PrintAndLog("Parity Passed");
		else
			PrintAndLog("Parity Failed");
	}
	return code;
}
/* Read the transmitted data of an EM4x50 tag from the graphbuffer
 * Format:
 *
 *  XXXXXXXX [row parity bit (even)] <- 8 bits plus parity
 *  XXXXXXXX [row parity bit (even)] <- 8 bits plus parity
 *  XXXXXXXX [row parity bit (even)] <- 8 bits plus parity
 *  XXXXXXXX [row parity bit (even)] <- 8 bits plus parity
 *  CCCCCCCC                         <- column parity bits
 *  0                                <- stop bit
 *  LW                               <- Listen Window
 *
 * This pattern repeats for every block of data being transmitted.
 * Transmission starts with two Listen Windows (LW - a modulated
 * pattern of 320 cycles each (32/32/128/64/64)).
 *
 * Note that this data may or may not be the UID. It is whatever data
 * is stored in the blocks defined in the control word First and Last
 * Word Read values. UID is stored in block 32.
 */
 //completed by Marshmellow
int EM4x50Read(const char *Cmd, bool verbose)
{
	uint8_t fndClk[] = {8,16,32,40,50,64,128};
	int clk = 0;
	int invert = 0;
	int tol = 0;
	int i, j, startblock, skip, block, start, end, low, high, minClk;
	bool complete = false;
	int tmpbuff[MAX_GRAPH_TRACE_LEN / 64];
	uint32_t Code[6];
	char tmp[6];
	char tmp2[20];
	int phaseoff;
	high = low = 0;
	memset(tmpbuff, 0, sizeof(tmpbuff));

	// get user entry if any
	sscanf(Cmd, "%i %i", &clk, &invert);

	// first get high and low values
	for (i = 0; i < GraphTraceLen; i++) {
		if (GraphBuffer[i] > high)
			high = GraphBuffer[i];
		else if (GraphBuffer[i] < low)
			low = GraphBuffer[i];
	}

	i = 0;
	j = 0;
	minClk = 255;
	// get to first full low to prime loop and skip incomplete first pulse
	while ((GraphBuffer[i] < high) && (i < GraphTraceLen))
		++i;
	while ((GraphBuffer[i] > low) && (i < GraphTraceLen))
		++i;
	skip = i;

	// populate tmpbuff buffer with pulse lengths
	while (i < GraphTraceLen) {
		// measure from low to low
		while ((GraphBuffer[i] > low) && (i < GraphTraceLen))
			++i;
		start= i;
		while ((GraphBuffer[i] < high) && (i < GraphTraceLen))
			++i;
		while ((GraphBuffer[i] > low) && (i < GraphTraceLen))
			++i;
		if (j>=(MAX_GRAPH_TRACE_LEN/64)) {
			break;
		}
		tmpbuff[j++]= i - start;
		if (i-start < minClk && i < GraphTraceLen) {
			minClk = i - start;
		}
	}
	// set clock
	if (!clk) {
		for (uint8_t clkCnt = 0; clkCnt<7; clkCnt++) {
			tol = fndClk[clkCnt]/8;
			if (minClk >= fndClk[clkCnt]-tol && minClk <= fndClk[clkCnt]+1) {
				clk=fndClk[clkCnt];
				break;
			}
		}
		if (!clk) return 0;
	} else tol = clk/8;

	// look for data start - should be 2 pairs of LW (pulses of clk*3,clk*2)
	start = -1;
	for (i= 0; i < j - 4 ; ++i) {
		skip += tmpbuff[i];
		if (tmpbuff[i] >= clk*3-tol && tmpbuff[i] <= clk*3+tol)  //3 clocks
			if (tmpbuff[i+1] >= clk*2-tol && tmpbuff[i+1] <= clk*2+tol)  //2 clocks
				if (tmpbuff[i+2] >= clk*3-tol && tmpbuff[i+2] <= clk*3+tol) //3 clocks
					if (tmpbuff[i+3] >= clk-tol)  //1.5 to 2 clocks - depends on bit following
					{
						start= i + 4;
						break;
					}
	}
	startblock = i + 4;

	// skip over the remainder of LW
	skip += tmpbuff[i+1] + tmpbuff[i+2] + clk;
	if (tmpbuff[i+3]>clk)
		phaseoff = tmpbuff[i+3]-clk;
	else
		phaseoff = 0;
	// now do it again to find the end
	end = skip;
	for (i += 3; i < j - 4 ; ++i) {
		end += tmpbuff[i];
		if (tmpbuff[i] >= clk*3-tol && tmpbuff[i] <= clk*3+tol)  //3 clocks
			if (tmpbuff[i+1] >= clk*2-tol && tmpbuff[i+1] <= clk*2+tol)  //2 clocks
				if (tmpbuff[i+2] >= clk*3-tol && tmpbuff[i+2] <= clk*3+tol) //3 clocks
					if (tmpbuff[i+3] >= clk-tol)  //1.5 to 2 clocks - depends on bit following
					{
						complete= true;
						break;
					}
	}
	end = i;
	// report back
	if (verbose || g_debugMode) {
		if (start >= 0) {
			PrintAndLog("\nNote: one block = 50 bits (32 data, 12 parity, 6 marker)");
		}	else {
			PrintAndLog("No data found!, clock tried:%d",clk);
			PrintAndLog("Try again with more samples.");
			PrintAndLog("  or after a 'data askedge' command to clean up the read");
			return 0;
		}
	} else if (start < 0) return 0;
	start = skip;
	snprintf(tmp2, sizeof(tmp2),"%d %d 1000 %d", clk, invert, clk*47);
	// save GraphBuffer - to restore it later
	save_restoreGB(GRAPH_SAVE);
	// get rid of leading crap
	snprintf(tmp, sizeof(tmp), "%i", skip);
	CmdLtrim(tmp);
	bool pTest;
	bool AllPTest = true;
	// now work through remaining buffer printing out data blocks
	block = 0;
	i = startblock;
	while (block < 6) {
		if (verbose || g_debugMode) PrintAndLog("\nBlock %i:", block);
		skip = phaseoff;

		// look for LW before start of next block
		for ( ; i < j - 4 ; ++i) {
			skip += tmpbuff[i];
			if (tmpbuff[i] >= clk*3-tol && tmpbuff[i] <= clk*3+tol)
				if (tmpbuff[i+1] >= clk-tol)
					break;
		}
		if (i >= j-4) break; //next LW not found
		skip += clk;
		if (tmpbuff[i+1]>clk)
			phaseoff = tmpbuff[i+1]-clk;
		else
			phaseoff = 0;
		i += 2;
		if (ASKDemod(tmp2, false, false, 1) < 1) {
			save_restoreGB(GRAPH_RESTORE);
			return 0;
		}
		//set DemodBufferLen to just one block
		DemodBufferLen = skip/clk;
		//test parities
		pTest = EM_ByteParityTest(DemodBuffer,DemodBufferLen,5,9,0);
		pTest &= EM_EndParityTest(DemodBuffer,DemodBufferLen,5,9,0);
		AllPTest &= pTest;
		//get output
		Code[block] = OutputEM4x50_Block(DemodBuffer,DemodBufferLen,verbose, pTest);
		if (g_debugMode) PrintAndLog("\nskipping %d samples, bits:%d", skip, skip/clk);
		//skip to start of next block
		snprintf(tmp,sizeof(tmp),"%i",skip);
		CmdLtrim(tmp);
		block++;
		if (i >= end) break; //in case chip doesn't output 6 blocks
	}
	//print full code:
	if (verbose || g_debugMode || AllPTest){
		if (!complete) {
			PrintAndLog("*** Warning!");
			PrintAndLog("Partial data - no end found!");
			PrintAndLog("Try again with more samples.");
		}
		PrintAndLog("Found data at sample: %i - using clock: %i", start, clk);
		end = block;
		for (block=0; block < end; block++){
			PrintAndLog("Block %d: %08x",block,Code[block]);
		}
		if (AllPTest) {
			PrintAndLog("Parities Passed");
		} else {
			PrintAndLog("Parities Failed");
			PrintAndLog("Try cleaning the read samples with 'data askedge'");
		}
	}

	//restore GraphBuffer
	save_restoreGB(GRAPH_RESTORE);
	return (int)AllPTest;
}

int CmdEM4x50Read(const char *Cmd)
{
	return EM4x50Read(Cmd, true);
}

//**************** Start of EM4x05/EM4x69 Code ************************
int usage_lf_em_read(void) {
	PrintAndLog("Read EM4x05/EM4x69.  Tag must be on antenna. ");
	PrintAndLog("");
	PrintAndLog("Usage:  lf em 4x05readword [h] <address> <pwd>");
	PrintAndLog("Options:");
	PrintAndLog("       h         - this help");
	PrintAndLog("       address   - memory address to read. (0-15)");
	PrintAndLog("       pwd       - password (hex) (optional)");
	PrintAndLog("samples:");
	PrintAndLog("      lf em 4x05readword 1");
	PrintAndLog("      lf em 4x05readword 1 11223344");
	return 0;
}

// for command responses from em4x05 or em4x69
// download samples from device and copy them to the Graphbuffer
bool downloadSamplesEM() {
	// 8 bit preamble + 32 bit word response (max clock (128) * 40bits = 5120 samples)
	uint8_t got[6000];
	if (!GetFromBigBuf(got, sizeof(got), 0, NULL, 4000, true)) {
		PrintAndLog("command execution time out");
		return false;
	}
	setGraphBuf(got, sizeof(got));
	return true;
}

bool EM4x05testDemodReadData(uint32_t *word, bool readCmd) {
	// em4x05/em4x69 command response preamble is 00001010
	// skip first two 0 bits as they might have been missed in the demod
	uint8_t preamble[] = {0,0,1,0,1,0};
	size_t startIdx = 0;

	// set size to 20 to only test first 14 positions for the preamble or less if not a read command
	size_t size = (readCmd) ? 20 : 11;
	// sanity check
	size = (size > DemodBufferLen) ? DemodBufferLen : size;
	// test preamble
	if ( !preambleSearchEx(DemodBuffer, preamble, sizeof(preamble), &size, &startIdx, true) ) {
		if (g_debugMode) PrintAndLog("DEBUG: Error - EM4305 preamble not found :: %d", startIdx);
		return false;
	}
	// if this is a readword command, get the read bytes and test the parities
	if (readCmd) {
		if (!EM_EndParityTest(DemodBuffer + startIdx + sizeof(preamble), 45, 5, 9, 0)) {
			if (g_debugMode) PrintAndLog("DEBUG: Error - End Parity check failed");
			return false;
		}
		// test for even parity bits and remove them. (leave out the end row of parities so 36 bits)
		if ( removeParity(DemodBuffer, startIdx + sizeof(preamble),9,0,36) == 0 ) {
			if (g_debugMode) PrintAndLog("DEBUG: Error - Parity not detected");
			return false;
		}

		setDemodBuf(DemodBuffer, 32, 0);
		//setClockGrid(0,0);

		*word = bytebits_to_byteLSBF(DemodBuffer, 32);
	}
	return true;
}

// FSK, PSK, ASK/MANCHESTER, ASK/BIPHASE, ASK/DIPHASE
// should cover 90% of known used configs
// the rest will need to be manually demoded for now...
int demodEM4x05resp(uint32_t *word, bool readCmd) {
	int ans = 0;

	// test for FSK wave (easiest to 99% ID)
	if (GetFskClock("", false, false)) {
		//valid fsk clocks found
		ans = FSKrawDemod("0 0", false);
		if (!ans) {
			if (g_debugMode) PrintAndLog("DEBUG: Error - EM4305: FSK Demod failed, ans: %d", ans);
		} else {
			if (EM4x05testDemodReadData(word, readCmd)) {
				return 1;
			}
		}
	}
	// PSK clocks should be easy to detect ( but difficult to demod a non-repeating pattern... )
	ans = GetPskClock("", false, false);
	if (ans>0) {
		//try psk1
		ans = PSKDemod("0 0 6", false);
		if (!ans) {
			if (g_debugMode) PrintAndLog("DEBUG: Error - EM4305: PSK1 Demod failed, ans: %d", ans);
		} else {
			if (EM4x05testDemodReadData(word, readCmd)) {
				return 1;
			} else {
				//try psk2
				psk1TOpsk2(DemodBuffer, DemodBufferLen);
				if (EM4x05testDemodReadData(word, readCmd)) {
					return 1;
				}
			}
			//try psk1 inverted
			ans = PSKDemod("0 1 6", false);
			if (!ans) {
				if (g_debugMode) PrintAndLog("DEBUG: Error - EM4305: PSK1 Demod failed, ans: %d", ans);
			} else {
				if (EM4x05testDemodReadData(word, readCmd)) {
					return 1;
				} else {
					//try psk2
					psk1TOpsk2(DemodBuffer, DemodBufferLen);
					if (EM4x05testDemodReadData(word, readCmd)) {
						return 1;
					}
				}
			}
		}
	}

	// manchester is more common than biphase... try first
	bool stcheck = false;
	// try manchester - NOTE: ST only applies to T55x7 tags.
	ans = ASKDemod_ext("0,0,1", false, false, 1, &stcheck);
	if (!ans) {
		if (g_debugMode) PrintAndLog("DEBUG: Error - EM4305: ASK/Manchester Demod failed, ans: %d", ans);
	} else {
		if (EM4x05testDemodReadData(word, readCmd)) {
			return 1;
		}
	}

	//try biphase
	ans = ASKbiphaseDemod("0 0 1", false);
	if (!ans) {
		if (g_debugMode) PrintAndLog("DEBUG: Error - EM4305: ASK/biphase Demod failed, ans: %d", ans);
	} else {
		if (EM4x05testDemodReadData(word, readCmd)) {
			return 1;
		}
	}

	//try diphase (differential biphase or inverted)
	ans = ASKbiphaseDemod("0 1 1", false);
	if (!ans) {
		if (g_debugMode) PrintAndLog("DEBUG: Error - EM4305: ASK/biphase Demod failed, ans: %d", ans);
	} else {
		if (EM4x05testDemodReadData(word, readCmd)) {
			return 1;
		}
	}

	return -1;
}

int EM4x05ReadWord_ext(uint8_t addr, uint32_t pwd, bool usePwd, uint32_t *wordData) {
	UsbCommand c = {CMD_EM4X_READ_WORD, {addr, pwd, usePwd}};
	clearCommandBuffer();
	SendCommand(&c);
	UsbCommand resp;
	if (!WaitForResponseTimeout(CMD_ACK, &resp, 2500)){
		PrintAndLog("Command timed out");
		return -1;
	}
	if ( !downloadSamplesEM() ) {
		return -1;
	}
	int testLen = (GraphTraceLen < 1000) ? GraphTraceLen : 1000;
	if (graphJustNoise(GraphBuffer, testLen)) {
		return -1;
	}
	//attempt demod:
	return demodEM4x05resp(wordData, true);
}

int EM4x05ReadWord(uint8_t addr, uint32_t pwd, bool usePwd) {
	uint32_t wordData = 0;
	int success = EM4x05ReadWord_ext(addr, pwd, usePwd, &wordData);
	if (success == 1)
		PrintAndLog("%s Address %02d | %08X", (addr>13) ? "Lock":" Got",addr,wordData);
	else
		PrintAndLog("Read Address %02d | failed",addr);

	return success;
}

int CmdEM4x05ReadWord(const char *Cmd) {
	uint8_t addr;
	uint32_t pwd;
	bool usePwd = false;
	uint8_t ctmp = param_getchar(Cmd, 0);
	if ( strlen(Cmd) == 0 || ctmp == 'H' || ctmp == 'h' ) return usage_lf_em_read();

	addr = param_get8ex(Cmd, 0, 50, 10);
	// for now use default input of 1 as invalid (unlikely 1 will be a valid password...)
	pwd =  param_get32ex(Cmd, 1, 1, 16);

	if ( (addr > 15) ) {
		PrintAndLog("Address must be between 0 and 15");
		return 1;
	}
	if ( pwd == 1 ) {
		PrintAndLog("Reading address %02u", addr);
	}	else {
		usePwd = true;
		PrintAndLog("Reading address %02u | password %08X", addr, pwd);
	}

	return EM4x05ReadWord(addr, pwd, usePwd);
}

int usage_lf_em_dump(void) {
	PrintAndLog("Dump EM4x05/EM4x69.  Tag must be on antenna. ");
	PrintAndLog("");
	PrintAndLog("Usage:  lf em 4x05dump [h] <pwd>");
	PrintAndLog("Options:");
	PrintAndLog("       h         - this help");
	PrintAndLog("       pwd       - password (hex) (optional)");
	PrintAndLog("samples:");
	PrintAndLog("      lf em 4x05dump");
	PrintAndLog("      lf em 4x05dump 11223344");
	return 0;
}

int CmdEM4x05dump(const char *Cmd) {
	uint8_t addr = 0;
	uint32_t pwd;
	bool usePwd = false;
	uint8_t ctmp = param_getchar(Cmd, 0);
	if ( ctmp == 'H' || ctmp == 'h' ) return usage_lf_em_dump();

	// for now use default input of 1 as invalid (unlikely 1 will be a valid password...)
	pwd = param_get32ex(Cmd, 0, 1, 16);

	if ( pwd != 1 ) {
		usePwd = true;
	}
	int success = 1;
	for (; addr < 16; addr++) {
		if (addr == 2) {
			if (usePwd) {
				PrintAndLog(" PWD Address %02u | %08X",addr,pwd);
			} else {
				PrintAndLog(" PWD Address 02 | cannot read");
			}
		} else {
			success &= EM4x05ReadWord(addr, pwd, usePwd);
		}
	}

	return success;
}


int usage_lf_em_write(void) {
	PrintAndLog("Write EM4x05/EM4x69.  Tag must be on antenna. ");
	PrintAndLog("");
	PrintAndLog("Usage:  lf em 4x05writeword [h] a <address> d <data> p <pwd> [s] [i]");
	PrintAndLog("Options:");
	PrintAndLog("       h           - this help");
	PrintAndLog("       a <address> - memory address to write to. (0-15)");
	PrintAndLog("       d <data>    - data to write (hex)");
	PrintAndLog("       p <pwd>     - password (hex) (optional)");
	PrintAndLog("       s           - swap the data bit order before write");
	PrintAndLog("       i           - invert the data bits before write");
	PrintAndLog("samples:");
	PrintAndLog("      lf em 4x05writeword a 5 d 11223344");
	PrintAndLog("      lf em 4x05writeword a 5 p deadc0de d 11223344 s i");
	return 0;
}

// note: em4x05 doesn't have a way to invert data output so we must invert the data prior to writing
//         it if invertion is needed. (example FSK2a vs FSK)
//       also em4x05 requires swapping word data when compared to the data used for t55xx chips.
int EM4x05WriteWord(uint8_t addr, uint32_t data, uint32_t pwd, bool usePwd, bool swap, bool invert) {
	if (swap) data = SwapBits(data, 32);

	if (invert) data ^= 0xFFFFFFFF;

	if ( (addr > 15) ) {
		PrintAndLog("Address must be between 0 and 15");
		return -1;
	}
	if ( !usePwd ) {
		PrintAndLog("Writing address %d data %08X", addr, data);
	} else {
		PrintAndLog("Writing address %d data %08X using password %08X", addr, data, pwd);
	}

	uint16_t flag = (addr << 8 ) | usePwd;

	UsbCommand c = {CMD_EM4X_WRITE_WORD, {flag, data, pwd}};
	clearCommandBuffer();
	SendCommand(&c);
	UsbCommand resp;
	if (!WaitForResponseTimeout(CMD_ACK, &resp, 2000)){
		PrintAndLog("Error occurred, device did not respond during write operation.");
		return -1;
	}
	if ( !downloadSamplesEM() ) {
		return -1;
	}
	//check response for 00001010 for write confirmation!
	//attempt demod:
	uint32_t dummy = 0;
	int result = demodEM4x05resp(&dummy,false);
	if (result == 1) {
		PrintAndLog("Write Verified");
	} else {
		PrintAndLog("Write could not be verified");
	}
	return result;
}

int CmdEM4x05WriteWord(const char *Cmd) {
	bool errors = false;
	bool usePwd = false;
	uint32_t data = 0xFFFFFFFF;
	uint32_t pwd = 0xFFFFFFFF;
	bool swap = false;
	bool invert = false;
	uint8_t addr = 16; // default to invalid address
	bool gotData = false;
	char cmdp = 0;
	while(param_getchar(Cmd, cmdp) != 0x00)
	{
		switch(param_getchar(Cmd, cmdp))
		{
		case 'h':
		case 'H':
			return usage_lf_em_write();
		case 'a':
		case 'A':
			addr = param_get8ex(Cmd, cmdp+1, 16, 10);
			cmdp += 2;
			break;
		case 'd':
		case 'D':
			data = param_get32ex(Cmd, cmdp+1, 0, 16);
			gotData = true;
			cmdp += 2;
			break;
		case 'i':
		case 'I':
			invert = true;
			cmdp++;
			break;
		case 'p':
		case 'P':
			pwd = param_get32ex(Cmd, cmdp+1, 1, 16);
			if (pwd == 1) {
				PrintAndLog("invalid pwd");
				errors = true;
			}
			usePwd = true;
			cmdp += 2;
			break;
		case 's':
		case 'S':
			swap = true;
			cmdp++;
			break;
		default:
			PrintAndLog("Unknown parameter '%c'", param_getchar(Cmd, cmdp));
			errors = true;
			break;
		}
		if(errors) break;
	}
	//Validations
	if(errors) return usage_lf_em_write();

	if ( strlen(Cmd) == 0 ) return usage_lf_em_write();

	if (!gotData) {
		PrintAndLog("You must enter the data you want to write");
		return usage_lf_em_write();
	}
	return EM4x05WriteWord(addr, data, pwd, usePwd, swap, invert);
}

void printEM4x05config(uint32_t wordData) {
	uint16_t datarate = EM4x05_GET_BITRATE(wordData);
	uint8_t encoder = ((wordData >> 6) & 0xF);
	char enc[14];
	memset(enc,0,sizeof(enc));

	uint8_t PSKcf = (wordData >> 10) & 0x3;
	char cf[10];
	memset(cf,0,sizeof(cf));
	uint8_t delay = (wordData >> 12) & 0x3;
	char cdelay[33];
	memset(cdelay,0,sizeof(cdelay));
	uint8_t numblks = EM4x05_GET_NUM_BLOCKS(wordData);
	uint8_t LWR = numblks+5-1; //last word read
	switch (encoder) {
		case 0: snprintf(enc,sizeof(enc),"NRZ"); break;
		case 1: snprintf(enc,sizeof(enc),"Manchester"); break;
		case 2: snprintf(enc,sizeof(enc),"Biphase"); break;
		case 3: snprintf(enc,sizeof(enc),"Miller"); break;
		case 4: snprintf(enc,sizeof(enc),"PSK1"); break;
		case 5: snprintf(enc,sizeof(enc),"PSK2"); break;
		case 6: snprintf(enc,sizeof(enc),"PSK3"); break;
		case 7: snprintf(enc,sizeof(enc),"Unknown"); break;
		case 8: snprintf(enc,sizeof(enc),"FSK1"); break;
		case 9: snprintf(enc,sizeof(enc),"FSK2"); break;
		default: snprintf(enc,sizeof(enc),"Unknown"); break;
	}

	switch (PSKcf) {
		case 0: snprintf(cf,sizeof(cf),"RF/2"); break;
		case 1: snprintf(cf,sizeof(cf),"RF/8"); break;
		case 2: snprintf(cf,sizeof(cf),"RF/4"); break;
		case 3: snprintf(cf,sizeof(cf),"unknown"); break;
	}

	switch (delay) {
		case 0: snprintf(cdelay, sizeof(cdelay),"no delay"); break;
		case 1: snprintf(cdelay, sizeof(cdelay),"BP/8 or 1/8th bit period delay"); break;
		case 2: snprintf(cdelay, sizeof(cdelay),"BP/4 or 1/4th bit period delay"); break;
		case 3: snprintf(cdelay, sizeof(cdelay),"no delay"); break;
	}
	uint8_t readLogin = (wordData & EM4x05_READ_LOGIN_REQ)>>18;
	uint8_t readHKL = (wordData & EM4x05_READ_HK_LOGIN_REQ)>>19;
	uint8_t writeLogin = (wordData & EM4x05_WRITE_LOGIN_REQ)>>20;
	uint8_t writeHKL = (wordData & EM4x05_WRITE_HK_LOGIN_REQ)>>21;
	uint8_t raw = (wordData & EM4x05_READ_AFTER_WRITE)>>22;
	uint8_t disable = (wordData & EM4x05_DISABLE_ALLOWED)>>23;
	uint8_t rtf = (wordData & EM4x05_READER_TALK_FIRST)>>24;
	uint8_t pigeon = (wordData & (1<<26))>>26;
	PrintAndLog("ConfigWord: %08X (Word 4)\n", wordData);
	PrintAndLog("Config Breakdown:");
	PrintAndLog(" Data Rate:  %02u | RF/%u", wordData & 0x3F, datarate);
	PrintAndLog("   Encoder:   %u | %s", encoder, enc);
	PrintAndLog("    PSK CF:   %u | %s", PSKcf, cf);
	PrintAndLog("     Delay:   %u | %s", delay, cdelay);
	PrintAndLog(" LastWordR:  %02u | Address of last word for default read - meaning %u blocks are output", LWR, numblks);
	PrintAndLog(" ReadLogin:   %u | Read Login is %s", readLogin, readLogin ? "Required" : "Not Required");
	PrintAndLog("   ReadHKL:   %u | Read Housekeeping Words Login is %s", readHKL, readHKL ? "Required" : "Not Required");
	PrintAndLog("WriteLogin:   %u | Write Login is %s", writeLogin, writeLogin ? "Required" : "Not Required");
	PrintAndLog("  WriteHKL:   %u | Write Housekeeping Words Login is %s", writeHKL, writeHKL ? "Required" : "Not Required");
	PrintAndLog("    R.A.W.:   %u | Read After Write is %s", raw, raw ? "On" : "Off");
	PrintAndLog("   Disable:   %u | Disable Command is %s", disable, disable ? "Accepted" : "Not Accepted");
	PrintAndLog("    R.T.F.:   %u | Reader Talk First is %s", rtf, rtf ? "Enabled" : "Disabled");
	PrintAndLog("    Pigeon:   %u | Pigeon Mode is %s\n", pigeon, pigeon ? "Enabled" : "Disabled");
}

void printEM4x05info(uint8_t chipType, uint8_t cap, uint16_t custCode, uint32_t serial) {
	switch (chipType) {
		case 9: PrintAndLog("\n Chip Type:   %u | EM4305", chipType); break;
		case 4: PrintAndLog(" Chip Type:   %u | Unknown", chipType); break;
		case 2: PrintAndLog(" Chip Type:   %u | EM4469", chipType); break;
		//add more here when known
		default: PrintAndLog(" Chip Type:   %u Unknown", chipType); break;
	}

	switch (cap) {
		case 3: PrintAndLog("  Cap Type:   %u | 330pF",cap); break;
		case 2: PrintAndLog("  Cap Type:   %u | %spF",cap, (chipType==2)? "75":"210"); break;
		case 1: PrintAndLog("  Cap Type:   %u | 250pF",cap); break;
		case 0: PrintAndLog("  Cap Type:   %u | no resonant capacitor",cap); break;
		default: PrintAndLog("  Cap Type:   %u | unknown",cap); break;
	}

	PrintAndLog(" Cust Code: %03u | %s", custCode, (custCode == 0x200) ? "Default": "Unknown");
	if (serial != 0) {
		PrintAndLog("\n  Serial #: %08X\n", serial);
	}
}

void printEM4x05ProtectionBits(uint32_t wordData) {
	for (uint8_t i = 0; i < 15; i++) {
		PrintAndLog("      Word:  %02u | %s", i, (((1 << i) & wordData ) || i < 2) ? "Is Write Locked" : "Is Not Write Locked");
		if (i==14) {
			PrintAndLog("      Word:  %02u | %s", i+1, (((1 << i) & wordData ) || i < 2) ? "Is Write Locked" : "Is Not Write Locked");
		}
	}
}

//quick test for EM4x05/EM4x69 tag
bool EM4x05Block0Test(uint32_t *wordData) {
	if (EM4x05ReadWord_ext(0,0,false,wordData) == 1) {
		return true;
	}
	return false;
}

int CmdEM4x05info(const char *Cmd) {
	//uint8_t addr = 0;
	uint32_t pwd;
	uint32_t wordData = 0;
  	bool usePwd = false;
	uint8_t ctmp = param_getchar(Cmd, 0);
	if ( ctmp == 'H' || ctmp == 'h' ) return usage_lf_em_dump();

	// for now use default input of 1 as invalid (unlikely 1 will be a valid password...)
	pwd = param_get32ex(Cmd, 0, 1, 16);

	if ( pwd != 1 ) {
		usePwd = true;
	}

	// read word 0 (chip info)
	// block 0 can be read even without a password.
	if ( !EM4x05Block0Test(&wordData) )
		return -1;

	uint8_t chipType = (wordData >> 1) & 0xF;
	uint8_t cap = (wordData >> 5) & 3;
	uint16_t custCode = (wordData >> 9) & 0x3FF;

	// read word 1 (serial #) doesn't need pwd
	wordData = 0;
	if (EM4x05ReadWord_ext(1, 0, false, &wordData) != 1) {
		//failed, but continue anyway...
	}
	printEM4x05info(chipType, cap, custCode, wordData);

	// read word 4 (config block)
	// needs password if one is set
	wordData = 0;
	if ( EM4x05ReadWord_ext(4, pwd, usePwd, &wordData) != 1 ) {
		//failed
		PrintAndLog("Config block read failed - might be password protected.");
		return 0;
	}
	printEM4x05config(wordData);

	// read word 14 and 15 to see which is being used for the protection bits
	wordData = 0;
	if ( EM4x05ReadWord_ext(14, pwd, usePwd, &wordData) != 1 ) {
		//failed
		return 0;
	}
	// if status bit says this is not the used protection word
	if (!(wordData & 0x8000)) {
		if ( EM4x05ReadWord_ext(15, pwd, usePwd, &wordData) != 1 ) {
			//failed
			return 0;
		}
	}
	if (!(wordData & 0x8000)) {
		//something went wrong
		return 0;
	}
	printEM4x05ProtectionBits(wordData);

	return 1;
}


static command_t CommandTable[] =
{
	{"help",      CmdHelp, 1, "This help"},
	{"410xread",  CmdEMdemodASK, 0, "[findone] -- Extract ID from EM410x tag (option 0 for continuous loop, 1 for only 1 tag)"},
	{"410xdemod", CmdAskEM410xDemod,  1, "[clock] [invert<0|1>] [maxErr] -- Demodulate an EM410x tag from GraphBuffer (args optional)"},
	{"410xsim",   CmdEM410xSim, 0, "<UID> [clock rate] -- Simulate EM410x tag"},
	{"410xbrute",   CmdEM410xBrute, 0, "ids.txt [d (delay in ms)] [c (clock rate)] -- Reader bruteforce attack by simulating EM410x tags"},
	{"410xwatch", CmdEM410xWatch, 0, "['h'] -- Watches for EM410x 125/134 kHz tags (option 'h' for 134)"},
	{"410xspoof", CmdEM410xWatchnSpoof, 0, "['h'] --- Watches for EM410x 125/134 kHz tags, and replays them. (option 'h' for 134)" },
	{"410xwrite", CmdEM410xWrite, 0, "<UID> <'0' T5555> <'1' T55x7> [clock rate] -- Write EM410x UID to T5555(Q5) or T55x7 tag, optionally setting clock rate"},
	{"4x05dump",  CmdEM4x05dump, 0, "(pwd) -- Read EM4x05/EM4x69 all word data"},
	{"4x05info",  CmdEM4x05info, 0, "(pwd) -- Get info from EM4x05/EM4x69 tag"},
	{"4x05readword",  CmdEM4x05ReadWord, 0, "<Word> (pwd) -- Read EM4x05/EM4x69 word data"},
	{"4x05writeword", CmdEM4x05WriteWord, 0, "<Word> <data> (pwd) -- Write EM4x05/EM4x69 word data"},
	{"4x50read",  CmdEM4x50Read, 1, "demod data from EM4x50 tag from the graph buffer"},
	{NULL, NULL, 0, NULL}
};

int CmdLFEM4X(const char *Cmd)
{
	CmdsParse(CommandTable, Cmd);
	return 0;
}

int CmdHelp(const char *Cmd)
{
	CmdsHelp(CommandTable);
	return 0;
}
