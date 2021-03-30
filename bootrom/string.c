//-----------------------------------------------------------------------------
// Jonathan Westhues, Sept 2005
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Common string.h functions
//-----------------------------------------------------------------------------

#include "string.h"
#include <stdint.h>

void *memcpy(void *dest, const void *src, size_t len) {
	uint8_t *d = dest;
	const uint8_t *s = src;
	while((len--) > 0) {
		*d = *s;
		d++;
		s++;
	}
	return dest;
}


void *memset(void *dest, int c, size_t len) {
	uint8_t *d = dest;
	while((len--) > 0) {
		*d = c;
		d++;
	}
	return dest;
}


void *memmove(void *dest, const void *src, size_t len) {
	uint8_t *d = dest;
	const uint8_t *s = src;
	if (dest <= src) {
		while((len--) > 0) {
			*d = *s;
			d++;
			s++;
		} 
	} else {
		d = d + len - 1;
		s = s + len - 1;
		while((len--) > 0) {
			*d = *s;
			d--;
			s--;
		}
	}
	return dest;
}


int memcmp(const void *av, const void *bv, size_t len) {
	const uint8_t *a = av;
	const uint8_t *b = bv;

	while((len--) > 0) {
		if(*a != *b) {
			return *a - *b;
		}
		a++;
		b++;
	}
	return 0;
}


size_t strlen(const char *str) {
	int l = 0;
	while(*str) {
		l++;
		str++;
	}
	return l;
}


char* strncat(char *dest, const char *src, size_t n) {
	unsigned int dest_len = strlen(dest);
	unsigned int i;

	for (i = 0 ; i < n && src[i] != '\0' ; i++)
		dest[dest_len + i] = src[i];
	dest[dest_len + i] = '\0';

	return dest;
}


char* strcat(char *dest, const char *src) {
	unsigned int dest_len = strlen(dest);
	unsigned int i;

	for (i = 0 ; src[i] != '\0' ; i++)
		dest[dest_len + i] = src[i];
	dest[dest_len + i] = '\0';

	return dest;
}
