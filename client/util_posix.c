//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// utilities requiring Posix library functions
//-----------------------------------------------------------------------------

#if !defined(_WIN32)
#define _POSIX_C_SOURCE	199309L			// need nanosleep()
#else
#include <windows.h>
#endif

#include "util_posix.h"
#include <stdint.h>
#include <time.h>


// Timer functions
#if !defined (_WIN32)
#include <errno.h>

static void nsleep(uint64_t n) {
  struct timespec timeout;
  timeout.tv_sec = n/1000000000;
  timeout.tv_nsec = n%1000000000;
  while (nanosleep(&timeout, &timeout) && errno == EINTR);
}

void msleep(uint32_t n) {
	nsleep(1000000 * n);
}
#endif // _WIN32

// a milliseconds timer for performance measurement
uint64_t msclock() {
#if defined(_WIN32)
    #include <sys/types.h>
    
    // WORKAROUND FOR MinGW (some versions - use if normal code does not compile)
    // It has no _ftime_s and needs explicit inclusion of timeb.h
    #include <sys/timeb.h>
    struct _timeb t;
    _ftime(&t);
    return 1000 * t.time + t.millitm;
    
    // NORMAL CODE (use _ftime_s)
	//struct _timeb t;
    //if (_ftime_s(&t)) {
	//	return 0;
	//} else {
	//	return 1000 * t.time + t.millitm;
	//}
#else
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (t.tv_sec * 1000 + t.tv_nsec / 1000000);
#endif
}

