#include <inttypes.h>
#include <stdbool.h>
#include "mfkey32.h"
#include "crapto1/crapto1.h"

// 32 bit recover key from 2 nonces
	
bool mfkey32(nonces_t data, uint64_t *outputkey) {
	struct Crypto1State *s,*t;
	uint64_t outkey = 0;
	uint64_t key=0;     // recovered key
	uint32_t uid     = data.cuid;
	uint32_t nt      = data.nonce;  // first tag challenge (nonce)
	uint32_t nr0_enc = data.nr;  // first encrypted reader challenge
	uint32_t ar0_enc = data.ar;  // first encrypted reader response
	uint32_t nr1_enc = data.nr2; // second encrypted reader challenge
	uint32_t ar1_enc = data.ar2; // second encrypted reader response
	bool isSuccess = false;
	uint8_t counter=0;

	s = lfsr_recovery32(ar0_enc ^ prng_successor(nt, 64), 0);

	for(t = s; t->odd | t->even; ++t) {
		lfsr_rollback_word(t, 0, 0);
		lfsr_rollback_word(t, nr0_enc, 1);
		lfsr_rollback_word(t, uid ^ nt, 0);
		crypto1_get_lfsr(t, &key);
		crypto1_word(t, uid ^ nt, 0);
		crypto1_word(t, nr1_enc, 1);
		if (ar1_enc == (crypto1_word(t, 0, 0) ^ prng_successor(nt, 64))) {
			outkey = key;
			counter++;
			if (counter==20) break;
		}
	}
	isSuccess = (counter == 1);
	*outputkey = ( isSuccess ) ? outkey : 0;
	crypto1_destroy(s);
	/* //un-comment to save all keys to a stats.txt file 
	FILE *fout;
	if ((fout = fopen("stats.txt","ab")) == NULL) { 
		PrintAndLog("Could not create file name stats.txt");
		return 1;
	}
	fprintf(fout, "mfkey32,%d,%08x,%d,%s,%04x%08x,%.0Lf\r\n", counter, data.cuid, data.sector, (data.keytype) ? "B" : "A", (uint32_t)(outkey>>32) & 0xFFFF,(uint32_t)(outkey&0xFFFFFFFF),(long double)t1);
	fclose(fout);
	*/
	return isSuccess;
}

bool tryMfk32_moebius(nonces_t data, uint64_t *outputkey) {
	struct Crypto1State *s, *t;
	uint64_t outkey  = 0;
	uint64_t key 	   = 0;			     // recovered key
	uint32_t uid     = data.cuid;
	uint32_t nt0     = data.nonce;  // first tag challenge (nonce)
	uint32_t nr0_enc = data.nr;  // first encrypted reader challenge
	uint32_t ar0_enc = data.ar; // first encrypted reader response
	uint32_t nt1     = data.nonce2; // second tag challenge (nonce)
	uint32_t nr1_enc = data.nr2; // second encrypted reader challenge
	uint32_t ar1_enc = data.ar2; // second encrypted reader response	
	bool isSuccess = false;
	int counter = 0;
	
	//PrintAndLog("Enter mfkey32_moebius");
	s = lfsr_recovery32(ar0_enc ^ prng_successor(nt0, 64), 0);
  
	for(t = s; t->odd | t->even; ++t) {
		lfsr_rollback_word(t, 0, 0);
		lfsr_rollback_word(t, nr0_enc, 1);
		lfsr_rollback_word(t, uid ^ nt0, 0);
		crypto1_get_lfsr(t, &key);
		
		crypto1_word(t, uid ^ nt1, 0);
		crypto1_word(t, nr1_enc, 1);
		if (ar1_enc == (crypto1_word(t, 0, 0) ^ prng_successor(nt1, 64))) {
			//PrintAndLog("Found Key: [%012" PRIx64 "]",key);
			outkey=key;
			++counter;
			if (counter==20)
				break;
		}
	}
	isSuccess	= (counter == 1);
	*outputkey = ( isSuccess ) ? outkey : 0;
	crypto1_destroy(s);
	/* // un-comment to output all keys to stats.txt
	FILE *fout;
	if ((fout = fopen("stats.txt","ab")) == NULL) { 
		PrintAndLog("Could not create file name stats.txt");
		return 1;
	}
	fprintf(fout, "moebius,%d,%08x,%d,%s,%04x%08x,%0.Lf\r\n", counter, data.cuid, data.sector, (data.keytype) ? "B" : "A", (uint32_t) (outkey>>32),(uint32_t)(outkey&0xFFFFFFFF),(long double)t1);
	fclose(fout);
	*/
	return isSuccess;
}


#if defined(STANDALONE_TOOL)
#include <stdio.h>
int main (int argc, char *argv[]) {

  nonces_t data;
  uint32_t ks2;     // keystream used to encrypt reader response
  uint64_t key;     // recovered key

  printf("MIFARE Classic key recovery - based on 32 bits of keystream\n");
  printf("Recover key from two 32-bit reader authentication answers only!\n\n");

  if (argc < 7) {
    printf(" syntax: %s <uid> <nt> <{nr_0}> <{ar_0}> <{nr_1}> <{ar_1}>\n\n",argv[0]);
    return 1;
  }

  sscanf(argv[1],"%x",&data.cuid);
  sscanf(argv[2],"%x",&data.nonce);
  sscanf(argv[3],"%x",&data.nr);
  sscanf(argv[4],"%x",&data.ar);
  sscanf(argv[5],"%x",&data.nr2);
  sscanf(argv[6],"%x",&data.ar2);

  printf("Recovering key for:\n");
  printf("    uid: %08x\n",data.cuid);
  printf("     nt: %08x\n",data.nonce);
  printf(" {nr_0}: %08x\n",data.nr);
  printf(" {ar_0}: %08x\n",data.ar);
  printf(" {nr_1}: %08x\n",data.nr2);
  printf(" {ar_1}: %08x\n",data.ar2);

	// Generate lfsr succesors of the tag challenge
  printf("\nLFSR succesors of the tag challenge:\n");
  printf("  nt': %08x\n",prng_successor(data.nonce, 64));
  printf(" nt'': %08x\n",prng_successor(data.nonce, 96));

  // Extract the keystream from the messages
  printf("\nKeystream used to generate {ar} and {at}:\n");
  ks2 = data.ar ^ prng_successor(data.nonce, 64);
  printf("  ks2: %08x\n",ks2);

  if (mfkey32(data, &key)) {
	  printf("Recovered key: %012" PRIx64 "\n", key);
  } else {
	  printf("Couldn't recover key.\n");
  }
	  
}
#endif