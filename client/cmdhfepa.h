//-----------------------------------------------------------------------------
// Copyright (C) 2012 Frederik Möllers
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Commands related to the German electronic Identification Card
//-----------------------------------------------------------------------------

#ifndef CMDHFEPA_H__
#define CMDHFEPA_H__
#include "comms.h"

int CmdHFEPA(pm3_connection* conn, const char *Cmd);

int CmdHFEPACollectPACENonces(pm3_connection* conn, const char *Cmd);

#endif // CMDHFEPA_H__
