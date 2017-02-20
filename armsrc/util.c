//-----------------------------------------------------------------------------
// Jonathan Westhues, Sept 2005
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Utility functions used in many places, not specific to any piece of code.
//-----------------------------------------------------------------------------

#include "proxmark3.h"
#include "util.h"
#include "string.h"
#include "apps.h"
#include "BigBuf.h"



void print_result(char *name, uint8_t *buf, size_t len) {
   uint8_t *p = buf;

   if ( len % 16 == 0 ) {
	   for(; p-buf < len; p += 16)
       Dbprintf("[%s:%d/%d] %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
				name,
				p-buf,
				len,
				p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]
	   );
   }
   else {
   for(; p-buf < len; p += 8)
       Dbprintf("[%s:%d/%d] %02x %02x %02x %02x %02x %02x %02x %02x", name, p-buf, len, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
   }
}

size_t nbytes(size_t nbits) {
	return (nbits >> 3)+((nbits % 8) > 0);
}

uint32_t SwapBits(uint32_t value, int nrbits) {
	int i;
	uint32_t newvalue = 0;
	for(i = 0; i < nrbits; i++) {
		newvalue ^= ((value >> i) & 1) << (nrbits - 1 - i);
	}
	return newvalue;
}

void num_to_bytes(uint64_t n, size_t len, uint8_t* dest)
{
	while (len--) {
		dest[len] = (uint8_t) n;
		n >>= 8;
	}
}

uint64_t bytes_to_num(uint8_t* src, size_t len)
{
	uint64_t num = 0;
	while (len--)
	{
		num = (num << 8) | (*src);
		src++;
	}
	return num;
}

// RotateLeft - Ultralight, Desfire
void rol(uint8_t *data, const size_t len){
    uint8_t first = data[0];
    for (size_t i = 0; i < len-1; i++) {
        data[i] = data[i+1];
    }
    data[len-1] = first;
}
void lsl (uint8_t *data, size_t len) {
    for (size_t n = 0; n < len - 1; n++) {
        data[n] = (data[n] << 1) | (data[n+1] >> 7);
    }
    data[len - 1] <<= 1;
}

int32_t le24toh (uint8_t data[3])
{
    return (data[2] << 16) | (data[1] << 8) | data[0];
}

void LEDsoff()
{
	LED_A_OFF();
	LED_B_OFF();
	LED_C_OFF();
	LED_D_OFF();
}

// LEDs: R(C) O(A) G(B) -- R(D) [1, 2, 4 and 8]
void LED(int led, int ms)
{
	if (led & LED_RED)
		LED_C_ON();
	if (led & LED_ORANGE)
		LED_A_ON();
	if (led & LED_GREEN)
		LED_B_ON();
	if (led & LED_RED2)
		LED_D_ON();

	if (!ms)
		return;

	SpinDelay(ms);

	if (led & LED_RED)
		LED_C_OFF();
	if (led & LED_ORANGE)
		LED_A_OFF();
	if (led & LED_GREEN)
		LED_B_OFF();
	if (led & LED_RED2)
		LED_D_OFF();
}


// Determine if a button is double clicked, single clicked,
// not clicked, or held down (for ms || 1sec)
// In general, don't use this function unless you expect a
// double click, otherwise it will waste 500ms -- use BUTTON_HELD instead
int BUTTON_CLICKED(int ms)
{
	// Up to 500ms in between clicks to mean a double click
	int ticks = (48000 * (ms ? ms : 1000)) >> 10;

	// If we're not even pressed, forget about it!
	if (!BUTTON_PRESS())
		return BUTTON_NO_CLICK;

	// Borrow a PWM unit for my real-time clock
	AT91C_BASE_PWMC->PWMC_ENA = PWM_CHANNEL(0);
	// 48 MHz / 1024 gives 46.875 kHz
	AT91C_BASE_PWMC_CH0->PWMC_CMR = PWM_CH_MODE_PRESCALER(10);
	AT91C_BASE_PWMC_CH0->PWMC_CDTYR = 0;
	AT91C_BASE_PWMC_CH0->PWMC_CPRDR = 0xffff;

	uint16_t start = AT91C_BASE_PWMC_CH0->PWMC_CCNTR;

	int letoff = 0;
	for(;;)
	{
		uint16_t now = AT91C_BASE_PWMC_CH0->PWMC_CCNTR;

		// We haven't let off the button yet
		if (!letoff)
		{
			// We just let it off!
			if (!BUTTON_PRESS())
			{
				letoff = 1;

				// reset our timer for 500ms
				start = AT91C_BASE_PWMC_CH0->PWMC_CCNTR;
				ticks = (48000 * (500)) >> 10;
			}

			// Still haven't let it off
			else
				// Have we held down a full second?
				if (now == (uint16_t)(start + ticks))
					return BUTTON_HOLD;
		}

		// We already let off, did we click again?
		else
			// Sweet, double click!
			if (BUTTON_PRESS())
				return BUTTON_DOUBLE_CLICK;

			// Have we ran out of time to double click?
			else
				if (now == (uint16_t)(start + ticks))
					// At least we did a single click
					return BUTTON_SINGLE_CLICK;

		WDT_HIT();
	}

	// We should never get here
	return BUTTON_ERROR;
}

// Determine if a button is held down
int BUTTON_HELD(int ms)
{
	// If button is held for one second
	int ticks = (48000 * (ms ? ms : 1000)) >> 10;

	// If we're not even pressed, forget about it!
	if (!BUTTON_PRESS())
		return BUTTON_NO_CLICK;

	// Borrow a PWM unit for my real-time clock
	AT91C_BASE_PWMC->PWMC_ENA = PWM_CHANNEL(0);
	// 48 MHz / 1024 gives 46.875 kHz
	AT91C_BASE_PWMC_CH0->PWMC_CMR = PWM_CH_MODE_PRESCALER(10);
	AT91C_BASE_PWMC_CH0->PWMC_CDTYR = 0;
	AT91C_BASE_PWMC_CH0->PWMC_CPRDR = 0xffff;

	uint16_t start = AT91C_BASE_PWMC_CH0->PWMC_CCNTR;

	for(;;)
	{
		uint16_t now = AT91C_BASE_PWMC_CH0->PWMC_CCNTR;

		// As soon as our button let go, we didn't hold long enough
		if (!BUTTON_PRESS())
			return BUTTON_SINGLE_CLICK;

		// Have we waited the full second?
		else
			if (now == (uint16_t)(start + ticks))
				return BUTTON_HOLD;

		WDT_HIT();
	}

	// We should never get here
	return BUTTON_ERROR;
}

// attempt at high resolution microsecond timer
// beware: timer counts in 21.3uS increments (1024/48Mhz)
void SpinDelayUs(int us)
{
	int ticks = (48*us) >> 10;

	// Borrow a PWM unit for my real-time clock
	AT91C_BASE_PWMC->PWMC_ENA = PWM_CHANNEL(0);
	// 48 MHz / 1024 gives 46.875 kHz
	AT91C_BASE_PWMC_CH0->PWMC_CMR = PWM_CH_MODE_PRESCALER(10);
	AT91C_BASE_PWMC_CH0->PWMC_CDTYR = 0;
	AT91C_BASE_PWMC_CH0->PWMC_CPRDR = 0xffff;

	uint16_t start = AT91C_BASE_PWMC_CH0->PWMC_CCNTR;

	for(;;) {
		uint16_t now = AT91C_BASE_PWMC_CH0->PWMC_CCNTR;
		if (now == (uint16_t)(start + ticks))
			return;

		WDT_HIT();
	}
}

void SpinDelay(int ms)
{
  // convert to uS and call microsecond delay function
	SpinDelayUs(ms*1000);
}

/* Similar to FpgaGatherVersion this formats stored version information
 * into a string representation. It takes a pointer to the struct version_information,
 * verifies the magic properties, then stores a formatted string, prefixed by
 * prefix in dst.
 */
void FormatVersionInformation(char *dst, int len, const char *prefix, void *version_information)
{
	struct version_information *v = (struct version_information*)version_information;
	dst[0] = 0;
	strncat(dst, prefix, len-1);
	if(v->magic != VERSION_INFORMATION_MAGIC) {
		strncat(dst, "Missing/Invalid version information\n", len - strlen(dst) - 1);
		return;
	}
	if(v->versionversion != 1) {
		strncat(dst, "Version information not understood\n", len - strlen(dst) - 1);
		return;
	}
	if(!v->present) {
		strncat(dst, "Version information not available\n", len - strlen(dst) - 1);
		return;
	}

	strncat(dst, v->gitversion, len - strlen(dst) - 1);
	if(v->clean == 0) {
		strncat(dst, "-unclean", len - strlen(dst) - 1);
	} else if(v->clean == 2) {
		strncat(dst, "-suspect", len - strlen(dst) - 1);
	}

	strncat(dst, " ", len - strlen(dst) - 1);
	strncat(dst, v->buildtime, len - strlen(dst) - 1);
	strncat(dst, "\n", len - strlen(dst) - 1);
}

//  -------------------------------------------------------------------------
//  timer lib
//  -------------------------------------------------------------------------
//  test procedure:
//
//	ti = GetTickCount();
//	SpinDelay(1000);
//	ti = GetTickCount() - ti;
//	Dbprintf("timer(1s): %d t=%d", ti, GetTickCount());

void StartTickCount()
{
	// This timer is based on the slow clock. The slow clock frequency is between 22kHz and 40kHz.
	// We can determine the actual slow clock frequency by looking at the Main Clock Frequency Register.
    uint16_t mainf = AT91C_BASE_PMC->PMC_MCFR & 0xffff;		// = 16 * main clock frequency (16MHz) / slow clock frequency
	// set RealTimeCounter divider to count at 1kHz:
	AT91C_BASE_RTTC->RTTC_RTMR = AT91C_RTTC_RTTRST | ((256000 + (mainf/2)) / mainf);
	// note: worst case precision is approx 2.5%
}

/*
* Get the current count.
*/
uint32_t RAMFUNC GetTickCount(){
	return AT91C_BASE_RTTC->RTTC_RTVR;// was * 2;
}

//  -------------------------------------------------------------------------
//  microseconds timer 
//  -------------------------------------------------------------------------
void StartCountUS()
{
	AT91C_BASE_PMC->PMC_PCER |= (0x1 << 12) | (0x1 << 13) | (0x1 << 14);
//	AT91C_BASE_TCB->TCB_BMR = AT91C_TCB_TC1XC1S_TIOA0;
	AT91C_BASE_TCB->TCB_BMR = AT91C_TCB_TC0XC0S_NONE | AT91C_TCB_TC1XC1S_TIOA0 | AT91C_TCB_TC2XC2S_NONE;

	// fast clock
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKDIS; // timer disable
	AT91C_BASE_TC0->TC_CMR = AT91C_TC_CLKS_TIMER_DIV3_CLOCK | // MCK(48MHz)/32 -- tick=1.5mks
														AT91C_TC_WAVE | AT91C_TC_WAVESEL_UP_AUTO | AT91C_TC_ACPA_CLEAR |
														AT91C_TC_ACPC_SET | AT91C_TC_ASWTRG_SET;
	AT91C_BASE_TC0->TC_RA = 1;
	AT91C_BASE_TC0->TC_RC = 0xBFFF + 1; // 0xC000
	
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS; // timer disable  
	AT91C_BASE_TC1->TC_CMR = AT91C_TC_CLKS_XC1; // from timer 0
	
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKEN;
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKEN;
	AT91C_BASE_TCB->TCB_BCR = 1;
	}

uint32_t RAMFUNC GetCountUS(){
	return (AT91C_BASE_TC1->TC_CV * 0x8000) + ((AT91C_BASE_TC0->TC_CV * 2) / 3); //was  /15) * 10);
}

static uint32_t GlobalUsCounter = 0;

uint32_t RAMFUNC GetDeltaCountUS(){
	uint32_t g_cnt = GetCountUS();
	uint32_t g_res = g_cnt - GlobalUsCounter;
	GlobalUsCounter = g_cnt;
	return g_res;
}


//  -------------------------------------------------------------------------
//  Timer for iso14443 commands. Uses ssp_clk from FPGA 
//  -------------------------------------------------------------------------
void StartCountSspClk()
{
	AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_TC0) | (1 << AT91C_ID_TC1) | (1 << AT91C_ID_TC2);  // Enable Clock to all timers
	AT91C_BASE_TCB->TCB_BMR = AT91C_TCB_TC0XC0S_TIOA1 		// XC0 Clock = TIOA1
							| AT91C_TCB_TC1XC1S_NONE 		// XC1 Clock = none
							| AT91C_TCB_TC2XC2S_TIOA0;		// XC2 Clock = TIOA0

	// configure TC1 to create a short pulse on TIOA1 when a rising edge on TIOB1 (= ssp_clk from FPGA) occurs:
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS; 				// disable TC1
	AT91C_BASE_TC1->TC_CMR = AT91C_TC_CLKS_TIMER_DIV1_CLOCK // TC1 Clock = MCK(48MHz)/2 = 24MHz
							| AT91C_TC_CPCSTOP				// Stop clock on RC compare
							| AT91C_TC_EEVTEDG_RISING		// Trigger on rising edge of Event
							| AT91C_TC_EEVT_TIOB			// Event-Source: TIOB1 (= ssp_clk from FPGA = 13,56MHz/16)
							| AT91C_TC_ENETRG				// Enable external trigger event
							| AT91C_TC_WAVESEL_UP	 		// Upmode without automatic trigger on RC compare
							| AT91C_TC_WAVE 				// Waveform Mode
							| AT91C_TC_AEEVT_SET 			// Set TIOA1 on external event
							| AT91C_TC_ACPC_CLEAR; 			// Clear TIOA1 on RC Compare
	AT91C_BASE_TC1->TC_RC = 0x04; 							// RC Compare value = 0x04

	// use TC0 to count TIOA1 pulses
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKDIS;				// disable TC0
	AT91C_BASE_TC0->TC_CMR = AT91C_TC_CLKS_XC0	 			// TC0 clock = XC0 clock = TIOA1
							| AT91C_TC_WAVE 				// Waveform Mode
							| AT91C_TC_WAVESEL_UP	 		// just count
							| AT91C_TC_ACPA_CLEAR 			// Clear TIOA0 on RA Compare
							| AT91C_TC_ACPC_SET; 			// Set TIOA0 on RC Compare
	AT91C_BASE_TC0->TC_RA = 1;								// RA Compare value = 1; pulse width to TC2
	AT91C_BASE_TC0->TC_RC = 0; 								// RC Compare value = 0; increment TC2 on overflow

	// use TC2 to count TIOA0 pulses (giving us a 32bit counter (TC0/TC2) clocked by ssp_clk)
	AT91C_BASE_TC2->TC_CCR = AT91C_TC_CLKDIS; 				// disable TC2  
	AT91C_BASE_TC2->TC_CMR = AT91C_TC_CLKS_XC2	 			// TC2 clock = XC2 clock = TIOA0
							| AT91C_TC_WAVE 				// Waveform Mode
							| AT91C_TC_WAVESEL_UP;	 		// just count
	
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKEN;				// enable TC0
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKEN;				// enable TC1
	AT91C_BASE_TC2->TC_CCR = AT91C_TC_CLKEN;				// enable TC2

	//
	// synchronize the counter with the ssp_frame signal. Note: FPGA must be in any iso14446 mode, otherwise the frame signal would not be present 
	//
	while(!(AT91C_BASE_PIOA->PIO_PDSR & GPIO_SSC_FRAME)); 	// wait for ssp_frame to go high (start of frame)
	while(AT91C_BASE_PIOA->PIO_PDSR & GPIO_SSC_FRAME); 		// wait for ssp_frame to be low
	while(!(AT91C_BASE_PIOA->PIO_PDSR & GPIO_SSC_CLK)); 	// wait for ssp_clk to go high
	// note: up to now two ssp_clk rising edges have passed since the rising edge of ssp_frame
	// it is now safe to assert a sync signal. This sets all timers to 0 on next active clock edge
	AT91C_BASE_TCB->TCB_BCR = 1;							// assert Sync (set all timers to 0 on next active clock edge)
	// at the next (3rd) ssp_clk rising edge, TC1 will be reset (and not generate a clock signal to TC0)
	// at the next (4th) ssp_clk rising edge, TC0 (the low word of our counter) will be reset. From now on,
	// whenever the last three bits of our counter go 0, we can be sure to be in the middle of a frame transfer.
	// (just started with the transfer of the 4th Bit).
	// The high word of the counter (TC2) will not reset until the low word (TC0) overflows. Therefore need to wait quite some time before
	// we can use the counter.
	while (AT91C_BASE_TC0->TC_CV < 0xFFF0);
}
void ResetSspClk(void) {
	//enable clock of timer and software trigger
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;
	AT91C_BASE_TC2->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;
	while (AT91C_BASE_TC2->TC_CV > 0);
}
uint32_t RAMFUNC GetCountSspClk(){
	uint32_t tmp_count;
	tmp_count = (AT91C_BASE_TC2->TC_CV << 16) | AT91C_BASE_TC0->TC_CV;
	if ((tmp_count & 0x0000ffff) == 0) { //small chance that we may have missed an increment in TC2
		return (AT91C_BASE_TC2->TC_CV << 16);
	} 
	else {
		return tmp_count;
	}
}

//  -------------------------------------------------------------------------
//  Timer for bitbanging,  or LF stuff when you need a very precis timer
//  1us = 1.5ticks
//  -------------------------------------------------------------------------
void StartTicks(void){
	//initialization of the timer
	// tc1 is higher 0xFFFF0000
	// tc0 is lower 0x0000FFFF
	AT91C_BASE_PMC->PMC_PCER |= (1 << AT91C_ID_TC0) | (1 << AT91C_ID_TC1);
	AT91C_BASE_TCB->TCB_BMR = AT91C_TCB_TC0XC0S_NONE | AT91C_TCB_TC1XC1S_TIOA0 | AT91C_TCB_TC2XC2S_NONE;
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKDIS;
	AT91C_BASE_TC0->TC_CMR = AT91C_TC_CLKS_TIMER_DIV3_CLOCK | // MCK(48MHz) / 32 
								AT91C_TC_WAVE | AT91C_TC_WAVESEL_UP_AUTO | AT91C_TC_ACPA_CLEAR |
								AT91C_TC_ACPC_SET | AT91C_TC_ASWTRG_SET;
	AT91C_BASE_TC0->TC_RA = 1;
	AT91C_BASE_TC0->TC_RC = 0; 

	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS;	// timer disable  
	AT91C_BASE_TC1->TC_CMR = AT91C_TC_CLKS_XC1; // from TC0
	
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;
	AT91C_BASE_TCB->TCB_BCR = 1;
	
	// wait until timer becomes zero.
	while (AT91C_BASE_TC1->TC_CV > 0);
}

// Wait - Spindelay in ticks.
// if called with a high number, this will trigger the WDT...
void WaitTicks(uint32_t ticks){
	if ( ticks == 0 ) return;
	ticks += GET_TICKS;	
	while (GET_TICKS < ticks);
}
// Wait / Spindelay in us (microseconds) 
// 1us = 1.5ticks.
void WaitUS(uint16_t us){
	if ( us == 0 ) return;
	WaitTicks(  (uint32_t)(us * 1.5) );
}
void WaitMS(uint16_t ms){
	if (ms == 0) return;
	WaitTicks( (uint32_t)(ms * 1500) );
}
// Starts Clock and waits until its reset
void ResetTicks(void){
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;
	while (AT91C_BASE_TC1->TC_CV > 0);
}
void ResetTimer(AT91PS_TC timer){
	timer->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;
	while(timer->TC_CV > 0) ;
}
// stop clock
void StopTicks(void){
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKDIS;
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS;	
}

static uint64_t next_random = 1;

/* Generates a (non-cryptographically secure) 32-bit random number.
 *
 * We don't have an implementation of the "rand" function or a clock to seed it
 * with, so we just call GetTickCount the first time to seed ourselves.
 */
uint32_t prand() {
	if (next_random == 1) {
		next_random = GetTickCount();
	}

	next_random = next_random * 6364136223846793005 + 1;
	return (uint32_t)(next_random >> 32) % 0xffffffff;
}

