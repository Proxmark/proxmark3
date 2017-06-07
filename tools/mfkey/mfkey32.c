#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "crapto1/crapto1.h"
#include "mfkey.h"
#include "util_posix.h"


// 32 bit recover key from 2 nonces
int main (int argc, char *argv[]) {

  nonces_t data;
  uint32_t ks2;     // keystream used to encrypt reader response
  uint64_t key;     // recovered key

  printf("MIFARE Classic key recovery - based on 32 bits of keystream\n");
  printf("Recover key from two 32-bit reader authentication answers only!\n\n");

  if (argc != 7 && argc != 8) {
    printf(" syntax: %s <uid> <nt0> <{nr_0}> <{ar_0}> [<nt1>] <{nr_1}> <{ar_1}>\n", argv[0]);
	printf("         (you may omit nt1 if it is equal to nt0)\n\n");
    return 1;
  }

  bool moebius_attack = (argc == 8);
  
  sscanf(argv[1],"%x",&data.cuid);
  sscanf(argv[2],"%x",&data.nonce);
  data.nonce2 = data.nonce;
  sscanf(argv[3],"%x",&data.nr);
  sscanf(argv[4],"%x",&data.ar);
  if (moebius_attack) {
	  sscanf(argv[5],"%x",&data.nonce2);
	  sscanf(argv[6],"%x",&data.nr2);
	  sscanf(argv[7],"%x",&data.ar2);
  } else {
	  sscanf(argv[5],"%x",&data.nr2);
	  sscanf(argv[6],"%x",&data.ar2);
  }	  

  printf("Recovering key for:\n");
  printf("    uid: %08x\n",data.cuid);
  printf("    nt0: %08x\n",data.nonce);
  printf(" {nr_0}: %08x\n",data.nr);
  printf(" {ar_0}: %08x\n",data.ar);
  printf("    nt1: %08x\n",data.nonce2);
  printf(" {nr_1}: %08x\n",data.nr2);
  printf(" {ar_1}: %08x\n",data.ar2);

  uint64_t start_time = msclock();
  
	// Generate lfsr succesors of the tag challenge
  printf("\nLFSR succesors of the tag challenge:\n");
  printf("  nt': %08x\n",prng_successor(data.nonce, 64));
  printf(" nt'': %08x\n",prng_successor(data.nonce, 96));

  // Extract the keystream from the messages
  printf("\nKeystream used to generate {ar} and {at}:\n");
  ks2 = data.ar ^ prng_successor(data.nonce, 64);
  printf("  ks2: %08x\n",ks2);

	bool success;
	if (moebius_attack) {
		success = mfkey32_moebius(data, &key);
	} else {
		success = mfkey32(data, &key);
	}
	
	if (success) {
		printf("Recovered key: %012" PRIx64 "\n", key);
	} else {
		printf("Couldn't recover key.\n");
	}

	printf("Time spent: %1.2f seconds\n", (float)(msclock() - start_time)/1000.0);
}
