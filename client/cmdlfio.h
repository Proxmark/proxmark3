// Low frequency Kantech commands
//-----------------------------------------------------------------------------

#ifndef CMDLFIO_H__
#define CMDLFIO_H__

#include "comms.h"

extern int CmdLFIO(pm3_connection* conn, const char *Cmd);
extern int CmdFSKdemodIO(pm3_connection* conn, const char *Cmd);
extern int CmdIOReadFSK(pm3_connection* conn, const char *Cmd);

#endif
