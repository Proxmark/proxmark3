//-----------------------------------------------------------------------------
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency Securakey tag commands
//-----------------------------------------------------------------------------
#ifndef CMDLFSECURAKEY_H__
#define CMDLFSECURAKEY_H__

int CmdLFSecurakey(const char *Cmd);
int CmdSecurakeyClone(const char *Cmd);
int CmdSecurakeySim(const char *Cmd);
int CmdSecurakeyRead(const char *Cmd);
int CmdSecurakeyDemod(const char *Cmd);

#endif

