//-----------------------------------------------------------------------------
// Copyright (C) 2012 Roel Verdult
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency Hitag support
//-----------------------------------------------------------------------------

#ifndef CMDLFHITAG_H__
#define CMDLFHITAG_H__

#include <stdint.h>
#include <stdbool.h>

extern int CmdLFHitag(const char *Cmd);
extern int CmdLFHitagReader(const char *Cmd);
extern bool getHitagUid(uint32_t *uid, bool quiet);

#endif
