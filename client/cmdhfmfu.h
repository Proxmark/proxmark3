#ifndef CMDHFMFU_H__
#define CMDHFMFU_H__

#include <stdint.h>

extern int CmdHFMFUltra(const char *Cmd);
extern uint32_t GetHF14AMfU_Type(void);
extern int ul_print_type(uint32_t tagtype, uint8_t spacer);

#endif
