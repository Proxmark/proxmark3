//-----------------------------------------------------------------------------
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// HitagS emulation (preliminary test version)
//
// (c) 2016 Oguzhan Cicek, Hendrik Schwartke, Ralf Spenneberg
//     <info@os-s.de>
//-----------------------------------------------------------------------------


#ifndef _HITAGS_H_
#define _HITAGS_H_

#include "hitag2.h"

typedef enum PROTO_STATE {READY=0,INIT,AUTHENTICATE,SELECTED,QUIET,TTF,FAIL} PSTATE;				//protocol-state
typedef enum TAG_STATE   {NO_OP=0,READING_PAGE,WRITING_PAGE_ACK,WRITING_PAGE_DATA,WRITING_BLOCK_DATA} TSATE;	//tag-state
typedef enum SOF_TYPE    {STANDARD=0,ADVANCED,FAST_ADVANCED,ONE,NO_BITS} stype;					//number of start-of-frame bits

struct hitagS_tag {
	PSTATE pstate; //protocol-state
	TSATE tstate;  //tag-state
	uint32_t uid;
	uint32_t pages[16][4];
	uint64_t key;
	byte_t pwdl0,pwdl1,pwdh0;
	//con0
	int max_page;
	stype mode;
	//con1
	bool auth;  //0=Plain 1=Auth
	bool TTFC;  //Transponder Talks first coding. 0=Manchester 1=Biphase
	int TTFDR;  //data rate in TTF Mode
	int TTFM;   //the number of pages that are sent to the RWD
	bool LCON;  //0=con1/2 read write 1=con1 read only and con2 OTP
	bool LKP;   //0=page2/3 read write 1=page2/3 read only in Plain mode and no access in authenticate mode
	//con2
	//0=read write 1=read only
	bool LCK7; //page4/5
	bool LCK6; //page6/7
	bool LCK5; //page8-11
	bool LCK4; //page12-15
	bool LCK3; //page16-23
	bool LCK2; //page24-31
	bool LCK1; //page32-47
	bool LCK0; //page48-63
} ;

#endif
