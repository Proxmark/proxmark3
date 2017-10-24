//-----------------------------------------------------------------------------
// Merlok - June 2011, 2012
// Gerhard de Koning Gans - May 2008
// Hagen Fritsch - June 2010
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Mifare Classic Card Simulation
//-----------------------------------------------------------------------------

#ifndef __MIFARESIM_H
#define __MIFARESIM_H

#include <stdint.h>

extern void Mifare1ksim(uint8_t flags, uint8_t exitAfterNReads, uint8_t arg2, uint8_t *datain);

#endif
