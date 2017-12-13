//-----------------------------------------------------------------------------
// Jonathan Westhues, Aug 2005
// Gerhard de Koning Gans, April 2008, May 2011
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// BigBuf and functions to allocate/free parts of it.
//-----------------------------------------------------------------------------

#ifndef __BIGBUF_H
#define __BIGBUF_H

#include <stdbool.h> // for bool
#include "common.h"  // for ramfunc

#define BIGBUF_SIZE				40000
#define MAX_FRAME_SIZE			256		// maximum allowed ISO14443 frame
#define MAX_PARITY_SIZE			((MAX_FRAME_SIZE + 7) / 8)
#define MAX_MIFARE_FRAME_SIZE	18		// biggest Mifare frame is answer to a read (one block = 16 Bytes) + 2 Bytes CRC
#define MAX_MIFARE_PARITY_SIZE	3		// need 18 parity bits for the 18 Byte above. 3 Bytes are enough to store these
#define CARD_MEMORY_SIZE		4096	
#define DMA_BUFFER_SIZE    		128

uint8_t *BigBuf_get_addr(void);
uint8_t *BigBuf_get_EM_addr(void);
uint16_t BigBuf_max_traceLen(void);
void BigBuf_Clear(void);
void BigBuf_Clear_ext(bool verbose);
void BigBuf_Clear_keep_EM(void);
void BigBuf_Clear_EM(void);
uint8_t *BigBuf_malloc(uint16_t);
void BigBuf_free(void);
void BigBuf_free_keep_EM(void);
void BigBuf_print_status(void);
uint16_t BigBuf_get_traceLen(void);
void clear_trace(void);
void set_tracing(bool enable);
bool get_tracing(void);
bool RAMFUNC LogTrace(const uint8_t *btBytes, uint16_t iLen, uint32_t timestamp_start, uint32_t timestamp_end, uint8_t *parity, bool readerToTag);
int LogTraceHitag(const uint8_t * btBytes, int iBits, int iSamples, uint32_t dwParity, int bReader);
uint8_t emlSet(uint8_t *data, uint32_t offset, uint32_t length);
#endif /* __BIGBUF_H */
