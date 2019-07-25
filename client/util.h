//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// utilities
//-----------------------------------------------------------------------------

#ifndef UTIL_H__
#define UTIL_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef _MSC_VER
#define PACKED
#else
#define PACKED __attribute__((packed))
#endif

#ifndef ROTR
# define ROTR(x,n) (((uintmax_t)(x) >> (n)) | ((uintmax_t)(x) << ((sizeof(x) * 8) - (n))))
#endif
#ifndef MIN
# define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
# define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef arraylen
#define arraylen(x) (sizeof(x)/sizeof((x)[0]))
#endif

#define EVEN                        0
#define ODD                         1

#ifndef FILE_PATH_SIZE
#define FILE_PATH_SIZE           2000
#endif

#ifndef ARRAYLEN
# define ARRAYLEN(x) (sizeof(x)/sizeof((x)[0]))
#endif

#if defined(__linux__)	|| (__APPLE__)
# define _BLUE_(s) "\x1b[34m" #s "\x1b[0m "
#else
# define _BLUE_(s) #s " "
#endif

#if defined(__linux__)	|| (__APPLE__)
# define _RED_(s) "\x1b[31m" #s "\x1b[0m "
#else
# define _RED_(s) #s " "
#endif

#if defined(__linux__)	|| (__APPLE__)
# define _GREEN_(s) "\x1b[32m" #s "\x1b[0m "
#else
# define _GREEN_(s) #s " "
#endif

#if defined(__linux__)	|| (__APPLE__)
# define _YELLOW_(s) "\x1b[33m" #s "\x1b[0m "
#else
# define _YELLOW_(s) #s " "
#endif

#if defined(__linux__)	|| (__APPLE__)
# define _MAGENTA_(s) "\x1b[35m" #s "\x1b[0m "
#else
# define _MAGENTA_(s) #s " "
#endif

#if defined(__linux__)	|| (__APPLE__)
# define _CYAN_(s) "\x1b[36m" #s "\x1b[0m "
#else
# define _CYAN_(s) #s " "
#endif

extern int ukbhit(void);
#ifndef _WIN32
extern char getch(void);
#else
#include <conio.h>
#endif

extern void AddLogLine(char *fileName, char *extData, char *c);
extern void AddLogHex(char *fileName, char *extData, const uint8_t * data, const size_t len);
extern void AddLogUint64(char *fileName, char *extData, const uint64_t data);
extern void AddLogCurrentDT(char *fileName);
extern void FillFileNameByUID(char *fileName, uint8_t * uid, char *ext, int byteCount);

// fill buffer from structure [{uint8_t data, size_t length},...]
extern int FillBuffer(uint8_t *data, size_t maxDataLength, size_t *dataLength, ...);

extern bool CheckStringIsHEXValue(const char *value);
extern void hex_to_buffer(const uint8_t *buf, const uint8_t *hex_data, const size_t hex_len, 
	const size_t hex_max_len, const size_t min_str_len, const size_t spaces_between, bool uppercase);

extern char *sprint_hex(const uint8_t * data, const size_t len);
extern char *sprint_hex_inrow(const uint8_t *data, const size_t len);
extern char *sprint_hex_inrow_ex(const uint8_t *data, const size_t len, const size_t min_str_len);
extern char *sprint_bin(const uint8_t * data, const size_t len);
extern char *sprint_bin_break(const uint8_t *data, const size_t len, const uint8_t breaks);
extern char *sprint_ascii_ex(const uint8_t *data, const size_t len, const size_t min_str_len);
extern char *sprint_ascii(const uint8_t *data, const size_t len);

extern void num_to_bytes(uint64_t n, size_t len, uint8_t* dest);
extern uint64_t bytes_to_num(uint8_t* src, size_t len);
extern void num_to_bytebits(uint64_t	n, size_t len, uint8_t *dest);
extern void num_to_bytebitsLSBF(uint64_t n, size_t len, uint8_t *dest);
extern char *printBits(size_t const size, void const * const ptr);
extern char * printBitsPar(const uint8_t *b, size_t len);
extern uint32_t SwapBits(uint32_t value, int nrbits);
extern uint8_t *SwapEndian64(const uint8_t *src, const size_t len, const uint8_t blockSize);

extern int param_getlength(const char *line, int paramnum);
extern char param_getchar(const char *line, int paramnum);
extern char param_getchar_indx(const char *line, int indx, int paramnum);
extern int param_getptr(const char *line, int *bg, int *en, int paramnum);
extern uint8_t param_get8(const char *line, int paramnum);
extern uint8_t param_get8ex(const char *line, int paramnum, int deflt, int base);
extern uint32_t param_get32ex(const char *line, int paramnum, int deflt, int base);
extern uint64_t param_get64ex(const char *line, int paramnum, int deflt, int base);
extern uint8_t param_getdec(const char *line, int paramnum, uint8_t *destination);
extern uint8_t param_isdec(const char *line, int paramnum);
extern int param_gethex(const char *line, int paramnum, uint8_t * data, int hexcnt);
extern int param_gethex_ex(const char *line, int paramnum, uint8_t * data, int *hexcnt);
extern int param_gethex_to_eol(const char *line, int paramnum, uint8_t * data, int maxdatalen, int *datalen);
extern int param_getstr(const char *line, int paramnum, char * str, size_t buffersize);

extern int hextobinarray( char *target,  char *source);
extern int binarraytohex( char *target,  char *source,  int length);
extern uint8_t GetParity( uint8_t *string, uint8_t type,  int length);
extern void wiegand_add_parity(uint8_t *target, uint8_t *source, uint8_t length);

extern void xor(unsigned char *dst, unsigned char *src, size_t len);
extern void rol(uint8_t *data, const size_t len);

extern void clean_ascii(unsigned char *buf, size_t len);
void strcleanrn(char *buf, size_t len);
void strcreplace(char *buf, size_t len, char from, char to);
char *strmcopy(char *buf);

extern int num_CPUs(void);			// number of logical CPUs

#endif // UTIL_H__
