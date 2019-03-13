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
// Some code was copied from Hitag2.c
//-----------------------------------------------------------------------------

#ifndef HITAGS_H__
#define HITAGS_H__

#include <stdint.h>
#include <stdbool.h>
#include "hitag.h"

void ReadHitagSCmd(hitag_function htf, hitag_data* htd, uint64_t startPage, uint64_t tagMode, bool readBlock);
void SimulateHitagSTag(bool tag_mem_supplied, uint8_t* data);
void WritePageHitagS(hitag_function htf, hitag_data* htd, int page);
void check_challenges_cmd(bool file_given, uint8_t* data, uint64_t tagMode);

#endif