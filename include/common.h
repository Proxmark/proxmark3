//-----------------------------------------------------------------------------
// Hagen Fritsch - June 2010
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.

//-----------------------------------------------------------------------------
// Interlib Definitions
//-----------------------------------------------------------------------------

#ifndef COMMON_H__
#define COMMON_H__

typedef unsigned char byte_t;

#ifndef MIN
# define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
# define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef ABS
# define ABS(a) (((a) < 0) ? -(a) : (a))
#endif

#define RAMFUNC __attribute((long_call, section(".ramfunc")))

#endif
