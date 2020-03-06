//-----------------------------------------------------------------------------
// Jonathan Westhues, April 2006
// iZsh <izsh at fail0verflow.com>, 2014
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Routines to load the FPGA image, and then to configure the FPGA's major
// mode once it is configured.
//-----------------------------------------------------------------------------

#ifndef __FPGALOADER_H
#define __FPGALOADER_H

#include <stdint.h>
#include <stdbool.h>

void FpgaSendCommand(uint16_t cmd, uint16_t v);
void FpgaWriteConfWord(uint16_t v);
void FpgaDownloadAndGo(int bitstream_version);
void FpgaSetupSsc(uint16_t mode);
void SetupSpi(int mode);
bool FpgaSetupSscDma(uint8_t *buf, uint16_t sample_count);
void Fpga_print_status();
int FpgaGetCurrent();
void FpgaEnableTracing(void);
void FpgaDisableTracing(void);
#define FpgaDisableSscDma(void) AT91C_BASE_PDC_SSC->PDC_PTCR = AT91C_PDC_RXTDIS;
#define FpgaEnableSscDma(void) AT91C_BASE_PDC_SSC->PDC_PTCR = AT91C_PDC_RXTEN;
void SetAdcMuxFor(uint32_t whichGpio);

// definitions for multiple FPGA config files support
#define FPGA_BITSTREAM_LF 1
#define FPGA_BITSTREAM_HF 2

// Definitions for the FPGA commands.
#define FPGA_CMD_MASK                               0xF000
// BOTH
#define FPGA_CMD_SET_CONFREG                       (1<<12)
// LF
#define FPGA_CMD_SET_DIVISOR                       (2<<12)
#define FPGA_CMD_SET_EDGE_DETECT_THRESHOLD         (3<<12)
// HF
#define FPGA_CMD_TRACE_ENABLE                      (2<<12)

// Definitions for the FPGA configuration word.
#define FPGA_MAJOR_MODE_MASK                        0x01C0
// LF
#define FPGA_MAJOR_MODE_LF_ADC                      (0<<6)
#define FPGA_MAJOR_MODE_LF_EDGE_DETECT              (1<<6)
#define FPGA_MAJOR_MODE_LF_PASSTHRU                 (2<<6)
// HF
#define FPGA_MAJOR_MODE_HF_READER                   (0<<6)
#define FPGA_MAJOR_MODE_HF_SIMULATOR                (1<<6)
#define FPGA_MAJOR_MODE_HF_ISO14443A                (2<<6)
#define FPGA_MAJOR_MODE_HF_SNOOP                    (3<<6)
#define FPGA_MAJOR_MODE_HF_GET_TRACE                (4<<6)
// BOTH
#define FPGA_MAJOR_MODE_OFF                         (7<<6)

#define FPGA_MINOR_MODE_MASK                        0x003F
// Options for LF_ADC
#define FPGA_LF_ADC_READER_FIELD                    (1<<0)

// Options for LF_EDGE_DETECT
#define FPGA_LF_EDGE_DETECT_READER_FIELD            (1<<0)
#define FPGA_LF_EDGE_DETECT_TOGGLE_MODE             (2<<0)

// Options for the HF reader
#define FPGA_HF_READER_MODE_RECEIVE_IQ              (0<<0)
#define FPGA_HF_READER_MODE_RECEIVE_AMPLITUDE       (1<<0)
#define FPGA_HF_READER_MODE_RECEIVE_PHASE           (2<<0)
#define FPGA_HF_READER_MODE_SEND_FULL_MOD           (3<<0)
#define FPGA_HF_READER_MODE_SEND_SHALLOW_MOD        (4<<0)
#define FPGA_HF_READER_MODE_SNOOP_IQ                (5<<0)
#define FPGA_HF_READER_MODE_SNOOP_AMPLITUDE         (6<<0)
#define FPGA_HF_READER_MODE_SNOOP_PHASE             (7<<0)
#define FPGA_HF_READER_MODE_SEND_JAM                (8<<0)

#define FPGA_HF_READER_SUBCARRIER_848_KHZ           (0<<4)
#define FPGA_HF_READER_SUBCARRIER_424_KHZ           (1<<4)
#define FPGA_HF_READER_SUBCARRIER_212_KHZ           (2<<4)

// Options for the HF simulated tag, how to modulate
#define FPGA_HF_SIMULATOR_NO_MODULATION             (0<<0)
#define FPGA_HF_SIMULATOR_MODULATE_BPSK             (1<<0)
#define FPGA_HF_SIMULATOR_MODULATE_212K             (2<<0)
#define FPGA_HF_SIMULATOR_MODULATE_424K             (4<<0)
#define FPGA_HF_SIMULATOR_MODULATE_424K_8BIT        0x5//101

// Options for ISO14443A
#define FPGA_HF_ISO14443A_SNIFFER                   (0<<0)
#define FPGA_HF_ISO14443A_TAGSIM_LISTEN             (1<<0)
#define FPGA_HF_ISO14443A_TAGSIM_MOD                (2<<0)
#define FPGA_HF_ISO14443A_READER_LISTEN             (3<<0)
#define FPGA_HF_ISO14443A_READER_MOD                (4<<0)

#endif
