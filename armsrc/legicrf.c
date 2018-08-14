//-----------------------------------------------------------------------------
// (c) 2009 Henryk Plötz <henryk@ploetzli.ch>
//     2016 Iceman
//     2018 AntiCat (rwd rewritten)
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// LEGIC RF simulation code
//-----------------------------------------------------------------------------

#include "proxmark3.h"
#include "apps.h"
#include "util.h"
#include "string.h"

#include "legicrf.h"
#include "legic_prng.h"
#include "legic.h"
#include "crc.h"

static struct legic_frame {
	int bits;
	uint32_t data;
} current_frame;

static enum {
  STATE_DISCON,
  STATE_IV,
  STATE_CON,
} legic_state;

static crc_t    legic_crc;
static int      legic_read_count;
static uint32_t legic_prng_bc;
static uint32_t legic_prng_iv;

static int      legic_phase_drift;
static int      legic_frame_drift;
static int      legic_reqresp_drift;

AT91PS_TC timer;
AT91PS_TC prng_timer;

static legic_card_select_t card;/* metadata of currently selected card */

//-----------------------------------------------------------------------------
// Frame timing and pseudorandom number generator
//
// The Prng is forwarded every 100us (TAG_BIT_PERIOD), except when the reader is
// transmitting. In that case the prng has to be forwarded every bit transmitted:
//  - 60us for a 0 (RWD_TIME_0)
//  - 100us for a 1 (RWD_TIME_1)
//
// The data dependent timing makes writing comprehensible code significantly
// harder. The current aproach forwards the prng data based if there is data on
// air and time based, using GET_TICKS, during computational and wait periodes.
//
// To not have the necessity to calculate/guess exection time dependend timeouts
// tx_frame and rx_frame use a shared timestamp to coordinate tx and rx timeslots.
//-----------------------------------------------------------------------------

static uint32_t last_frame_end; /* ts of last bit of previews rx or tx frame */

#define RWD_TIME_PAUSE       30 /* 20us */
#define RWD_TIME_1          150 /* READER_TIME_PAUSE 20us off + 80us on = 100us */
#define RWD_TIME_0           90 /* READER_TIME_PAUSE 20us off + 40us on = 60us */
#define RWD_FRAME_WAIT      330 /* 220us from TAG frame end to READER frame start */
#define TAG_FRAME_WAIT      495 /* 330us from READER frame end to TAG frame start */
#define TAG_BIT_PERIOD      150 /* 100us */
#define TAG_WRITE_TIMEOUT    60 /* 40 * 100us (write should take at most 3.6ms) */

#define SIM_DIVISOR         586 /* prng_time/DIV count prng needs to be forwared */
#define SIM_SHIFT           900 /* prng_time+SHIFT shift of delayed start */
#define RWD_TIME_FUZZ        20 /* rather generous 13us, since the peak detector
                                /+ hysteresis fuzz quite a bit */

#define LEGIC_READ         0x01 /* Read Command */
#define LEGIC_WRITE        0x00 /* Write Command */

#define SESSION_IV         0x55 /* An arbitrary chose session IV, all shoud work */
#define OFFSET_LOG         1024 /* The largest Legic Prime card is 1k */
#define WRITE_LOWERLIMIT      4 /* UID and MCC are not writable */

#define INPUT_THRESHOLD       8 /* heuristically determined, lower values */
                                /* lead to detecting false ack during write */

#define FUZZ_EQUAL(value, target, fuzz) ((value) > ((target)-(fuzz)) && (value) < ((target)+(fuzz)))

//-----------------------------------------------------------------------------
// I/O interface abstraction (FPGA -> ARM)
//-----------------------------------------------------------------------------

static inline uint8_t rx_byte_from_fpga() {
  for(;;) {
    WDT_HIT();

    // wait for byte be become available in rx holding register
    if(AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_RXRDY)) {
      return AT91C_BASE_SSC->SSC_RHR;
    }
  }
}

//-----------------------------------------------------------------------------
// Demodulation (Reader)
//-----------------------------------------------------------------------------

// Returns am aproximated power measurement
//
// The FPGA running xcorrelation samples the subcarrier at ~13.56 MHz. The mode
// was initialy designed to receive BSPK/2-PSK. Hance, it reports an I/Q pair
// every 4.7us (8 bits i and 8 bits q). We use the average of 4 samples.
//
// The subcarrier amplitude can be calculated using Pythagoras sqrt(i^2 + q^2).
// To reduce CPU time the amplitude is approximated by using linear functions:
//   am = MAX(ABS(i),ABS(q)) + 1/2*MIN(ABS(i),ABSq))
//
// Note: The SSC receiver is never synchronized the calculation my be performed
// on a i/q pair from two subsequent correlations, but does not matter.
static inline int32_t sample_power() {
  int32_t cq = 0;
  int32_t ci = 0;

  for(size_t i = 0; i<4; i++) {
    cq += (int8_t)rx_byte_from_fpga();
    ci += (int8_t)rx_byte_from_fpga();
  }

  return (MAX(ABS(ci), ABS(cq)) + (MIN(ABS(ci), ABS(cq)) >> 1)) >> 2;
}

// Returns a demedulated bit
//
// An aproximated power measurement is available every 18.9us. The bit time
// is 100us. The code samples 5 times and uses the last (most stable) sample.
//
// Note: The demodulator would be drifting (18.9us * 5 != 100us), rx_frame_as_reader
// has a delay loop that aligns rx_bit_as_reader calls to the TAG tx timeslots.
static inline bool rx_bit_as_reader() {
  int32_t power;

  for(size_t i = 0; i<5; ++i) {
    power = sample_power();
  }

  return (power > INPUT_THRESHOLD);
}

//-----------------------------------------------------------------------------
// Modulation (Reader)
//
// I've tried to modulate the Legic specific pause-puls using ssc and the default
// ssc clock of 105.4 kHz (bit periode of 9.4us) - previous commit. However,
// the timing was not precise enough. By increasing the ssc clock this could
// be circumvented, but the adventage over bitbang would be little.
//-----------------------------------------------------------------------------

static inline void tx_bit_as_reader(bool bit) {
  // insert pause
  LOW(GPIO_SSC_DOUT);
  last_frame_end += RWD_TIME_PAUSE;
  while(GET_TICKS < last_frame_end) { };
  HIGH(GPIO_SSC_DOUT);

  // return to high, wait for bit periode to end
  last_frame_end += (bit ? RWD_TIME_1 : RWD_TIME_0) - RWD_TIME_PAUSE;
  while(GET_TICKS < last_frame_end) { };
}

//-----------------------------------------------------------------------------
// Frame Handling (Reader)
//
// The LEGIC RF protocol from card to reader does not include explicit frame
// start/stop information or length information. The reader must know beforehand
// how many bits it wants to receive.
// Notably: a card sending a stream of 0-bits is indistinguishable from no card
// present.
//-----------------------------------------------------------------------------

static void tx_frame_as_reader(uint32_t frame, uint8_t len) {
  FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER_TX);

  // wait for next tx timeslot
  last_frame_end += RWD_FRAME_WAIT;
  while(GET_TICKS < last_frame_end) { };

  // transmit frame, MSB first
  for(uint8_t i = 0; i < len; ++i) {
    bool bit = (frame >> i) & 0x01;
    tx_bit_as_reader(bit ^ legic_prng_get_bit());
    legic_prng_forward(1);
  };

  // add pause to mark end of the frame
  LOW(GPIO_SSC_DOUT);
  last_frame_end += RWD_TIME_PAUSE;
  while(GET_TICKS < last_frame_end) { };
  HIGH(GPIO_SSC_DOUT);
}

static uint32_t rx_frame_as_reader(uint8_t len) {
  FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER_RX_XCORR
                  | FPGA_HF_READER_RX_XCORR_848_KHZ
                  | FPGA_HF_READER_RX_XCORR_QUARTER_FREQ);

  // hold sampling until card is expected to respond
  last_frame_end += TAG_FRAME_WAIT;
  while(GET_TICKS < last_frame_end) { };

  uint32_t frame = 0;
  for(uint8_t i = 0; i < len; i++) {
    frame |= (rx_bit_as_reader() ^ legic_prng_get_bit()) << i;
    legic_prng_forward(1);

    // rx_bit_as_reader runs only 95us, resync to TAG_BIT_PERIOD
    last_frame_end += TAG_BIT_PERIOD;
  }

  return frame;
}

static bool rx_ack_as_reader() {
  // change fpga into rx mode
  FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER_RX_XCORR
                  | FPGA_HF_READER_RX_XCORR_848_KHZ
                  | FPGA_HF_READER_RX_XCORR_QUARTER_FREQ);

  // hold sampling until card is expected to respond
  last_frame_end += TAG_FRAME_WAIT;
  while(GET_TICKS < last_frame_end) { };

  uint32_t ack = 0;
  for(uint8_t i = 0; i < TAG_WRITE_TIMEOUT; ++i) {
    // sample bit
    ack = rx_bit_as_reader();
    legic_prng_forward(1);

    // rx_bit_as_reader runs only 95us, resync to TAG_BIT_PERIOD
    last_frame_end += TAG_BIT_PERIOD;

    // check if it was an ACK
    if(ack) {
      break;
    }
  }

  return ack;
}

//-----------------------------------------------------------------------------
// Legic Reader
//-----------------------------------------------------------------------------

int init_card(uint8_t cardtype, legic_card_select_t *p_card) {
  p_card->tagtype = cardtype;

  switch(p_card->tagtype) {
    case 0x0d:
      p_card->cmdsize = 6;
      p_card->addrsize = 5;
      p_card->cardsize = 22;
      break;
    case 0x1d:
      p_card->cmdsize = 9;
      p_card->addrsize = 8;
      p_card->cardsize = 256;
      break;
    case 0x3d:
      p_card->cmdsize = 11;
      p_card->addrsize = 10;
      p_card->cardsize = 1024;
      break;
    default:
      p_card->cmdsize = 0;
      p_card->addrsize = 0;
      p_card->cardsize = 0;
      return 2;
  }
  return 0;
}

static void init_reader(bool clear_mem) {
  // configure FPGA
  FpgaDownloadAndGo(FPGA_BITSTREAM_HF);
  FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER_RX_XCORR
                  | FPGA_HF_READER_RX_XCORR_848_KHZ
                  | FPGA_HF_READER_RX_XCORR_QUARTER_FREQ);
  SetAdcMuxFor(GPIO_MUXSEL_HIPKD);
  LED_D_ON();

  // configure SSC with defaults
  FpgaSetupSsc();

  // re-claim GPIO_SSC_DOUT as GPIO and enable output
  AT91C_BASE_PIOA->PIO_OER = GPIO_SSC_DOUT;
  AT91C_BASE_PIOA->PIO_PER = GPIO_SSC_DOUT;
  HIGH(GPIO_SSC_DOUT);

  // init crc calculator
  crc_init(&legic_crc, 4, 0x19 >> 1, 0x05, 0);

  // start us timer
  StartTicks();
}

// Setup reader to card connection
//
// The setup consists of a three way handshake:
//  - Transmit initialisation vector 7 bits
//  - Receive card type 6 bits
//  - Acknowledge frame 6 bits
static uint32_t setup_phase_reader(uint8_t iv) {
  // init coordination timestamp
  last_frame_end = GET_TICKS;

  // Switch on carrier and let the card charge for 5ms.
  last_frame_end += 7500;
  while(GET_TICKS < last_frame_end) { };

  legic_prng_init(0);
  tx_frame_as_reader(iv, 7);

  // configure iv
  legic_prng_init(iv);
  legic_prng_forward(2);

  // receive card type
  int32_t card_type = rx_frame_as_reader(6);
  legic_prng_forward(3);

  // send obsfuscated acknowledgment frame
  switch (card_type) {
    case 0x0D:
      tx_frame_as_reader(0x19, 6); // MIM22 | READCMD = 0x18 | 0x01
      break;
    case 0x1D:
    case 0x3D:
      tx_frame_as_reader(0x39, 6); // MIM256 | READCMD = 0x38 | 0x01
      break;
  }

  return card_type;
}

static uint8_t calc_crc4(uint16_t cmd, uint8_t cmd_sz, uint8_t value) {
  crc_clear(&legic_crc);
  crc_update(&legic_crc, (value << cmd_sz) | cmd, 8 + cmd_sz);
  return crc_finish(&legic_crc);
}

static int16_t read_byte(uint16_t index, uint8_t cmd_sz) {
  uint16_t cmd = (index << 1) | LEGIC_READ;

  // read one byte
  LED_B_ON();
  legic_prng_forward(2);
  tx_frame_as_reader(cmd, cmd_sz);
  legic_prng_forward(2);
  uint32_t frame = rx_frame_as_reader(12);
  LED_B_OFF();

  // split frame into data and crc
  uint8_t byte = BYTEx(frame, 0);
  uint8_t crc = BYTEx(frame, 1);

  // check received against calculated crc
  uint8_t calc_crc = calc_crc4(cmd, cmd_sz, byte);
  if(calc_crc != crc) {
    Dbprintf("!!! crc mismatch: %x != %x !!!",  calc_crc, crc);
    return -1;
  }

  legic_prng_forward(1);

  return byte;
}

// Transmit write command, wait until (3.6ms) the tag sends back an unencrypted
// ACK ('1' bit) and forward the prng time based.
bool write_byte(uint16_t index, uint8_t byte, uint8_t addr_sz) {
  uint32_t cmd = index << 1 | LEGIC_WRITE;          // prepare command
  uint8_t  crc = calc_crc4(cmd, addr_sz + 1, byte); // calculate crc
  cmd |= byte << (addr_sz + 1);                     // append value
  cmd |= (crc & 0xF) << (addr_sz + 1 + 8);          // and crc

  // send write command
  LED_C_ON();
  legic_prng_forward(2);
  tx_frame_as_reader(cmd, addr_sz + 1 + 8 + 4); // sz = addr_sz + cmd + data + crc
  legic_prng_forward(3);
  LED_C_OFF();

  // wait for ack
  return rx_ack_as_reader();
}

//-----------------------------------------------------------------------------
// Command Line Interface
//
// Only this functions are public / called from appmain.c
//-----------------------------------------------------------------------------
void LegicRfReader(int offset, int bytes) {
  uint8_t *BigBuf = BigBuf_get_addr();
  memset(BigBuf, 0, 1024);

  // configure ARM and FPGA
  init_reader(false);

  // establish shared secret and detect card type
  DbpString("Reading card ...");
  uint8_t card_type = setup_phase_reader(SESSION_IV);
  if(init_card(card_type, &card) != 0) {
    Dbprintf("No or unknown card found, aborting");
    goto OUT;
  }

  // if no argument is specified create full dump
  if(bytes == -1) {
    bytes = card.cardsize;
  }

  // do not read beyond card memory
  if(bytes + offset > card.cardsize) {
    bytes = card.cardsize - offset;
  }

  for(uint16_t i = 0; i < bytes; ++i) {
    int16_t byte = read_byte(offset + i, card.cmdsize);
    if(byte == -1) {
      Dbprintf("operation failed @ 0x%03.3x", bytes);
      goto OUT;
    }
    BigBuf[i] = byte;
  }

  // OK
  Dbprintf("Card (MIM %i) read, use 'hf legic decode' or", card.cardsize);
  Dbprintf("'data hexsamples %d' to view results", (bytes+7) & ~7);

OUT:
  FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
  LED_B_OFF();
  LED_C_OFF();
  LED_D_OFF();
  StopTicks();
}

void LegicRfWriter(int bytes, int offset) {
  uint8_t *BigBuf = BigBuf_get_addr();

  // configure ARM and FPGA
  init_reader(false);

  // uid is not writeable
  if(offset <= WRITE_LOWERLIMIT) {
    goto OUT;
  }

  // establish shared secret and detect card type
  Dbprintf("Writing 0x%02.2x - 0x%02.2x ...", offset, offset+bytes);
  uint8_t card_type = setup_phase_reader(SESSION_IV);
  if(init_card(card_type, &card) != 0) {
    Dbprintf("No or unknown card found, aborting");
    goto OUT;
  }

  // do not write beyond card memory
  if(bytes + offset > card.cardsize) {
    bytes = card.cardsize - offset;
  }

  // write in reverse order, only then is DCF (decremental field) writable
  while(bytes-- > 0 && !BUTTON_PRESS()) {
    if(!write_byte(bytes + offset, BigBuf[bytes], card.addrsize)) {
      Dbprintf("operation failed @ 0x%03.3x", bytes);
      goto OUT;
    }
  }

  // OK
  DbpString("Write successful");

OUT:
  FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
  LED_B_OFF();
  LED_C_OFF();
  LED_D_OFF();
  StopTicks();
}

//-----------------------------------------------------------------------------
// Legic Simulator
//-----------------------------------------------------------------------------

static void setup_timer(void)
{
	/* Set up Timer 1 to use for measuring time between pulses. Since we're bit-banging
	 * this it won't be terribly accurate but should be good enough.
	 */
	AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_TC1);
	timer = AT91C_BASE_TC1;
	timer->TC_CCR = AT91C_TC_CLKDIS;
	timer->TC_CMR = AT91C_TC_CLKS_TIMER_DIV3_CLOCK;
	timer->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;

	/* 
     * Set up Timer 2 to use for measuring time between frames in 
     * tag simulation mode. Runs 4x faster as Timer 1
	 */
    AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_TC2);
    prng_timer = AT91C_BASE_TC2;
    prng_timer->TC_CCR = AT91C_TC_CLKDIS;
	prng_timer->TC_CMR = AT91C_TC_CLKS_TIMER_DIV2_CLOCK;
    prng_timer->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;
}

/* Generate Keystream */
static uint32_t get_key_stream(int skip, int count)
{
  uint32_t key=0; int i;

  /* Use int to enlarge timer tc to 32bit */
  legic_prng_bc += prng_timer->TC_CV;
  prng_timer->TC_CCR = AT91C_TC_SWTRG;

  /* If skip == -1, forward prng time based */
  if(skip == -1) {
     i  = (legic_prng_bc+SIM_SHIFT)/SIM_DIVISOR; /* Calculate Cycles based on timer */
     i -= legic_prng_count(); /* substract cycles of finished frames */
     i -= count; /* substract current frame length, rewidn to bedinning */
     legic_prng_forward(i);
  } else {
     legic_prng_forward(skip);
  }

  /* Write Time Data into LOG */
  uint8_t *BigBuf = BigBuf_get_addr();
  if(count == 6) { i = -1; } else { i = legic_read_count; }
  BigBuf[OFFSET_LOG+128+i] = legic_prng_count();
  BigBuf[OFFSET_LOG+256+i*4]   = (legic_prng_bc >> 0) & 0xff;
  BigBuf[OFFSET_LOG+256+i*4+1] = (legic_prng_bc >> 8) & 0xff;
  BigBuf[OFFSET_LOG+256+i*4+2] = (legic_prng_bc >>16) & 0xff;
  BigBuf[OFFSET_LOG+256+i*4+3] = (legic_prng_bc >>24) & 0xff;
  BigBuf[OFFSET_LOG+384+i] = count;

  /* Generate KeyStream */
  for(i=0; i<count; i++) {
    key |= legic_prng_get_bit() << i;
    legic_prng_forward(1);
  }
  return key;
}

/* Send a frame in tag mode, the FPGA must have been set up by
 * LegicRfSimulate
 */
static void frame_send_tag(uint16_t response, int bits, int crypt)
{
   /* Bitbang the response */
   AT91C_BASE_PIOA->PIO_CODR = GPIO_SSC_DOUT;
   AT91C_BASE_PIOA->PIO_OER = GPIO_SSC_DOUT;
   AT91C_BASE_PIOA->PIO_PER = GPIO_SSC_DOUT;
        
   /* Use time to crypt frame */
   if(crypt) {
      legic_prng_forward(2); /* TAG_TIME_WAIT -> shift by 2 */
      int i; int key = 0;
      for(i=0; i<bits; i++) {
         key |= legic_prng_get_bit() << i;
         legic_prng_forward(1);
      }
      //Dbprintf("key = 0x%x", key);
      response = response ^ key;
   }

   /* Wait for the frame start */
   while(timer->TC_CV < (TAG_FRAME_WAIT - 30)) ;
       
   int i;
   for(i=0; i<bits; i++) {
      int nextbit = timer->TC_CV + TAG_BIT_PERIOD;
      int bit = response & 1;
      response = response >> 1;
      if(bit) {
         AT91C_BASE_PIOA->PIO_SODR = GPIO_SSC_DOUT;
      } else {
         AT91C_BASE_PIOA->PIO_CODR = GPIO_SSC_DOUT;
      }
      while(timer->TC_CV < nextbit) ;
   }
   AT91C_BASE_PIOA->PIO_CODR = GPIO_SSC_DOUT;
}

static void frame_append_bit(struct legic_frame * const f, int bit)
{
   if(f->bits >= 31) {
       return; /* Overflow, won't happen */
   }
   f->data |= (bit<<f->bits);
   f->bits++;
}

static void frame_clean(struct legic_frame * const f)
{
	f->data = 0;
	f->bits = 0;
}

/* Handle (whether to respond) a frame in tag mode */
static void frame_handle_tag(struct legic_frame const * const f)
{
	uint8_t *BigBuf = BigBuf_get_addr();

   /* First Part of Handshake (IV) */
   if(f->bits == 7) {
     if(f->data == SESSION_IV) {
        LED_C_ON();
        prng_timer->TC_CCR = AT91C_TC_SWTRG;
        legic_prng_init(f->data);
        frame_send_tag(0x3d, 6, 1); /* 0x3d^0x26 = 0x1b */
        legic_state = STATE_IV;
        legic_read_count = 0;
        legic_prng_bc = 0;
        legic_prng_iv = f->data;
 
        /* TIMEOUT */
        timer->TC_CCR = AT91C_TC_SWTRG;
        while(timer->TC_CV > 1);
        while(timer->TC_CV < 280);
        return;
      } else if((prng_timer->TC_CV % 50) > 40) {
        legic_prng_init(f->data);
        frame_send_tag(0x3d, 6, 1);
        SpinDelay(20);
        return;
     }
   }

   /* 0x19==??? */
   if(legic_state == STATE_IV) {
      if((f->bits == 6) && (f->data == (0x19 ^ get_key_stream(1, 6)))) {
         legic_state = STATE_CON;

         /* TIMEOUT */
         timer->TC_CCR = AT91C_TC_SWTRG;
         while(timer->TC_CV > 1);
         while(timer->TC_CV < 200);
         return;
      } else {
         legic_state = STATE_DISCON;
         LED_C_OFF();
         Dbprintf("0x19 - Frame: %03.3x", f->data);
         return;
      }
   }

   /* Read */
   if(f->bits == 11) {
      if(legic_state == STATE_CON) {
         int key   = get_key_stream(-1, 11); //legic_phase_drift, 11);
         int addr  = f->data ^ key; addr = addr >> 1;
         int data = BigBuf[addr];
         int hash = calc_crc4(addr, data, 11) << 8;
         BigBuf[OFFSET_LOG+legic_read_count] = (uint8_t)addr;
         legic_read_count++;

         //Dbprintf("Data:%03.3x, key:%03.3x, addr: %03.3x, read_c:%u", f->data, key, addr, read_c);
         legic_prng_forward(legic_reqresp_drift);

         frame_send_tag(hash | data, 12, 1);

         /* SHORT TIMEOUT */
         timer->TC_CCR = AT91C_TC_SWTRG;
         while(timer->TC_CV > 1);
         legic_prng_forward(legic_frame_drift);
         while(timer->TC_CV < 180);
         return;
      }
   }

   /* Write */
   if(f->bits == 23) {
      int key   = get_key_stream(-1, 23); //legic_frame_drift, 23);
      int addr  = f->data ^ key; addr = addr >> 1; addr = addr & 0x3ff;
      int data  = f->data ^ key; data = data >> 11; data = data & 0xff;

      /* write command */
      legic_state = STATE_DISCON;
      LED_C_OFF();
      Dbprintf("write - addr: %x, data: %x", addr, data);
      return;
   }

   if(legic_state != STATE_DISCON) {
      Dbprintf("Unexpected: sz:%u, Data:%03.3x, State:%u, Count:%u", f->bits, f->data, legic_state, legic_read_count);
      int i;
      Dbprintf("IV: %03.3x", legic_prng_iv);
      for(i = 0; i<legic_read_count; i++) {
         Dbprintf("Read Nb: %u, Addr: %u", i, BigBuf[OFFSET_LOG+i]);
      }

      for(i = -1; i<legic_read_count; i++) {
         uint32_t t;
         t  = BigBuf[OFFSET_LOG+256+i*4];
         t |= BigBuf[OFFSET_LOG+256+i*4+1] << 8;
         t |= BigBuf[OFFSET_LOG+256+i*4+2] <<16;
         t |= BigBuf[OFFSET_LOG+256+i*4+3] <<24;

         Dbprintf("Cycles: %u, Frame Length: %u, Time: %u", 
            BigBuf[OFFSET_LOG+128+i],
            BigBuf[OFFSET_LOG+384+i],
            t);
      }
   }
   legic_state = STATE_DISCON; 
   legic_read_count = 0;
   SpinDelay(10);
   LED_C_OFF();
   return; 
}

/* Read bit by bit untill full frame is received
 * Call to process frame end answer
 */
static void emit(int bit)
{
  if(bit == -1) {
     if(current_frame.bits <= 4) {
        frame_clean(&current_frame);
     } else {
        frame_handle_tag(&current_frame);
        frame_clean(&current_frame);
     }
     WDT_HIT();
  } else if(bit == 0) {
    frame_append_bit(&current_frame, 0);
  } else if(bit == 1) {
    frame_append_bit(&current_frame, 1);
  }
}

void LegicRfSimulate(int phase, int frame, int reqresp)
{
  /* ADC path high-frequency peak detector, FPGA in high-frequency simulator mode, 
   * modulation mode set to 212kHz subcarrier. We are getting the incoming raw
   * envelope waveform on DIN and should send our response on DOUT.
   *
   * The LEGIC RF protocol is pulse-pause-encoding from reader to card, so we'll
   * measure the time between two rising edges on DIN, and no encoding on the
   * subcarrier from card to reader, so we'll just shift out our verbatim data
   * on DOUT, 1 bit is 100us. The time from reader to card frame is still unclear,
   * seems to be 300us-ish.
   */

   if(phase < 0) {
      int i;
      for(i=0; i<=reqresp; i++) {
         legic_prng_init(SESSION_IV);
         Dbprintf("i=%u, key 0x%3.3x", i, get_key_stream(i, frame));
      }
      return;
   }

   legic_phase_drift = phase;
   legic_frame_drift = frame;
   legic_reqresp_drift = reqresp;

   FpgaDownloadAndGo(FPGA_BITSTREAM_HF);
   SetAdcMuxFor(GPIO_MUXSEL_HIPKD);
   FpgaSetupSsc();
   FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_SIMULATOR | FPGA_HF_SIMULATOR_MODULATE_212K);
   
   /* Bitbang the receiver */
   AT91C_BASE_PIOA->PIO_ODR = GPIO_SSC_DIN;
   AT91C_BASE_PIOA->PIO_PER = GPIO_SSC_DIN;
   
   setup_timer();
   crc_init(&legic_crc, 4, 0x19 >> 1, 0x5, 0);
   
   int old_level = 0;
   int active = 0;
   legic_state = STATE_DISCON;

   LED_B_ON();
   DbpString("Starting Legic emulator, press button to end");
   while(!BUTTON_PRESS()) {
      int level = !!(AT91C_BASE_PIOA->PIO_PDSR & GPIO_SSC_DIN);
      int time = timer->TC_CV;
                
      if(level != old_level) {
         if(level == 1) {
            timer->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;
            if(FUZZ_EQUAL(time, RWD_TIME_1, RWD_TIME_FUZZ)) {
               /* 1 bit */
               emit(1);
               active = 1;
               LED_A_ON();
            } else if(FUZZ_EQUAL(time, RWD_TIME_0, RWD_TIME_FUZZ)) {
               /* 0 bit */
               emit(0);
               active = 1;
               LED_A_ON();
            } else if(active) {
               /* invalid */
               emit(-1);
               active = 0;
               LED_A_OFF();
            }
         }
      }

      if(time >= (RWD_TIME_1+RWD_TIME_FUZZ) && active) {
         /* Frame end */
         emit(-1);
         active = 0;
         LED_A_OFF();
      }
                
      if(time >= (20*RWD_TIME_1) && (timer->TC_SR & AT91C_TC_CLKSTA)) {
         timer->TC_CCR = AT91C_TC_CLKDIS;
      }
                
      old_level = level;
      WDT_HIT();
   }
   DbpString("Stopped");
   LED_B_OFF();
   LED_A_OFF();
   LED_C_OFF();
}

