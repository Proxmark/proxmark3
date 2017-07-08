//-----------------------------------------------------------------------------
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency Presco tag commands
//-----------------------------------------------------------------------------
#ifndef CMDLFPRESCO_H__
#define CMDLFPRESCO_H__

#include <stdint.h>  //uint_32+
#include <stdbool.h> //bool
#include "comms.h"

int CmdLFPresco(pm3_connection* conn, const char *Cmd);
int CmdPrescoClone(pm3_connection* conn, const char *Cmd);
int CmdPrescoSim(pm3_connection* conn, const char *Cmd);

int GetWiegandFromPresco(const char *id, uint32_t *sitecode, uint32_t *usercode, uint32_t *fullcode, bool *Q5);

int usage_lf_presco_clone(void);
int usage_lf_presco_sim(void);
#endif

