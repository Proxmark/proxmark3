//-----------------------------------------------------------------------------
// Jonathan Westhues, Aug 2005
// Copyright (C) 2010 Hector Martin "marcan" <marcan@marcansoft.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Common string.h functions
//-----------------------------------------------------------------------------

#ifndef __STRING_H
#define __STRING_H

#include <stdint.h>
#include "util.h"

RAMFUNC void *memcpy(void *dest, const void *src, size_t len);
void *memset(void *dest, int c, size_t len);
void *memmove(void *dest, const void *src, size_t len);
RAMFUNC int memcmp(const void *av, const void *bv, size_t len);
size_t strlen(const char *str);
char *strncat(char *dest, const char *src, size_t n);
char *strcat(char *dest, const char *src);

#endif /* __STRING_H */
