//-----------------------------------------------------------------------------
// Copyright (C) 2015 piwi
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Implements a card only attack based on crypto text (encrypted nonces
// received during a nested authentication) only. Unlike other card only
// attacks this doesn't rely on implementation errors but only on the
// inherent weaknesses of the crypto1 cypher. Described in
//   Carlo Meijer, Roel Verdult, "Ciphertext-only Cryptanalysis on Hardened
//   Mifare Classic Cards" in Proceedings of the 22nd ACM SIGSAC Conference on 
//   Computer and Communications Security, 2015
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <pthread.h>
#include <math.h>
#include "proxmark3.h"
#include "cmdmain.h"
#include "ui.h"
#include "util.h"
#include "nonce2key/crapto1.h"


typedef struct noncelistentry {
	uint32_t nonce_enc;
	uint8_t par_enc;
	void *next;
} noncelistentry_t;

typedef struct noncelist {
	uint16_t num;
	uint16_t Sum;
	uint16_t Sum8_guess;
	float Sum8_prob;
	bool updated;
	noncelistentry_t *first;
} noncelist_t;


static uint32_t cuid;
static noncelist_t nonces[256];
static uint16_t first_byte_Sum = 0;
static uint16_t first_byte_num = 0;
static uint8_t best_first_byte;
static uint16_t guessed_Sum8;
static float guessed_Sum8_confidence;


typedef enum {
	EVEN_STATE = 0,
	ODD_STATE = 1
} odd_even_t;

#define MAX_PARTIAL_ODD_STATES	248801		// we know from pre-computing. Includes 0xffffffff as End Of List marker
#define MAX_PARTIAL_EVEN_STATES	124401		// dito

typedef struct {
	uint32_t *states;
	uint32_t len;
	uint32_t *index[256];
} partial_indexed_statelist_t;

typedef struct {
	uint32_t *states[2];
	uint32_t len[2];
	void* next;
} statelist_t;


partial_indexed_statelist_t partial_statelist_odd[17];
partial_indexed_statelist_t partial_statelist_even[17];

statelist_t *candidates = NULL;


static int add_nonce(uint32_t nonce_enc, uint8_t par_enc) 
{
	uint8_t first_byte = nonce_enc >> 24;
	noncelistentry_t *p1 = nonces[first_byte].first;
	noncelistentry_t *p2 = NULL;

	if (p1 == NULL) {			// first nonce with this 1st byte
		first_byte_num++;
		first_byte_Sum += parity((nonce_enc & 0xff000000) | (par_enc & 0x08) | 0x01); // 1st byte sum property. Note: added XOR 1
		// printf("Adding nonce 0x%08x, par_enc 0x%02x, parity(0x%08x) = %d\n", 
			// nonce_enc, 
			// par_enc, 
			// (nonce_enc & 0xff000000) | (par_enc & 0x08) |0x01, 
			// parity((nonce_enc & 0xff000000) | (par_enc & 0x08) | 0x01));
	}

	while (p1 != NULL && (p1->nonce_enc & 0x00ff0000) < (nonce_enc & 0x00ff0000)) {
		p2 = p1;
		p1 = p1->next;
	}
	
	if (p1 == NULL) { 																	// need to add at the end of the list
		if (p2 == NULL) { 			// list is empty yet. Add first entry.
			p2 = nonces[first_byte].first = malloc(sizeof(noncelistentry_t));
		} else {					// add new entry at end of existing list.
			p2 = p2->next = malloc(sizeof(noncelistentry_t));
		}
	} else if ((p1->nonce_enc & 0x00ff0000) != (nonce_enc & 0x00ff0000)) {				// found distinct 2nd byte. Need to insert.
		if (p2 == NULL) {			// need to insert at start of list
			p2 = nonces[first_byte].first = malloc(sizeof(noncelistentry_t));
		} else {
			p2 = p2->next = malloc(sizeof(noncelistentry_t));
		}
	} else {											// we have seen this 2nd byte before. Nothing to add or insert. 
		return (0);
	}

	// add or insert new data
	p2->next = p1;
	p2->nonce_enc = nonce_enc;
	p2->par_enc = par_enc;

	nonces[first_byte].num++;
	nonces[first_byte].Sum += parity((nonce_enc & 0x00ff0000) | (par_enc & 0x04) | 0x01); // 2nd byte sum property. Note: added XOR 1
	nonces[first_byte].updated = true;   // indicates that we need to recalculate the Sum(a8) probability for this first byte

	return (1);				// new nonce added
}


static uint16_t SumPropertyOdd(uint32_t odd_state)
{ 
	uint16_t oddsum = 0;
	for (uint16_t j = 0; j < 16; j++) {
		uint32_t oddstate = odd_state;
		uint16_t part_sum = 0;
		for (uint16_t i = 0; i < 5; i++) {
			part_sum ^= filter(oddstate);
			oddstate = (oddstate << 1) | ((j >> (3-i)) & 0x01) ;
		}
		oddsum += part_sum;
	}
	return oddsum;
}


static uint16_t SumPropertyEven(uint32_t even_state)
{
	uint16_t evensum = 0;
	for (uint16_t j = 0; j < 16; j++) {
		uint32_t evenstate = even_state;
		uint16_t part_sum = 0;
		for (uint16_t i = 0; i < 4; i++) {
			evenstate = (evenstate << 1) | ((j >> (3-i)) & 0x01) ;
			part_sum ^= filter(evenstate);
		}
		evensum += part_sum;
	}
	return evensum;
}


static uint16_t SumProperty(struct Crypto1State *s)
{
	uint16_t sum_odd = SumPropertyOdd(s->odd);
	uint16_t sum_even = SumPropertyEven(s->even);
	return (sum_odd*(16-sum_even) + (16-sum_odd)*sum_even);
}


static double p_hypergeometric(uint16_t N, uint16_t K, uint16_t n, uint16_t k) 
{
	// for efficient computation we are using the recursive definition
	//						(K-k+1) * (n-k+1)
	// P(X=k) = P(X=k-1) * --------------------
	//						   k * (N-K-n+k)
	// and
	//           (N-K)*(N-K-1)*...*(N-K-n+1)
	// P(X=0) = -----------------------------
	//               N*(N-1)*...*(N-n+1)

	if (n-k > N-K || k > K) return 0.0;	// avoids log(x<=0) in calculation below
	if (k == 0) {
		// use logarithms to avoid overflow with huge factorials (double type can only hold 170!)
		double log_result = 0.0;
		for (int16_t i = N-K; i >= N-K-n+1; i--) {
			log_result += log(i);
		} 
		for (int16_t i = N; i >= N-n+1; i--) {
			log_result -= log(i);
		}
		return exp(log_result);
	} else {
		if (n-k == N-K) {	// special case. The published recursion below would fail with a divide by zero exception
			double log_result = 0.0;
			for (int16_t i = k+1; i <= n; i++) {
				log_result += log(i);
			}
			for (int16_t i = K+1; i <= N; i++) {
				log_result -= log(i);
			}
			return exp(log_result);
		} else { 			// recursion
			return (p_hypergeometric(N, K, n, k-1) * (K-k+1) * (n-k+1) / (k * (N-K-n+k)));
		}
	}
}
	
	
static float sum_probability(uint16_t K, uint16_t n, uint16_t k)
{
	const uint16_t N = 256;
	
	const float p[257] = {
		0.0290, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 
		0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
		0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 
		0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
		0.0083, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 
		0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
		0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 
		0.0006, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
		0.0339, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 
		0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
		0.0048, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 
		0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
		0.0934, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 
		0.0119, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
		0.0489, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 
		0.0602, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
		0.4180, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 
		0.0602, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
		0.0489, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 
		0.0119, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
		0.0934, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 
		0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
		0.0048, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 
		0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
		0.0339, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 
		0.0006, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
		0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 
		0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
		0.0083, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 
		0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
		0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
		0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
		0.02900 };	

		if (k > K || p[K] == 0.0) return 0.0;

		double p_T_is_k_when_S_is_K = p_hypergeometric(N, K, n, k);
		double p_S_is_K = p[K];
		double p_T_is_k = 0;
		for (uint16_t i = 0; i <= 256; i++) {
			if (p[i] != 0.0) {
				p_T_is_k += p[i] * p_hypergeometric(N, i, n, k);
			}
		}
		return(p_T_is_k_when_S_is_K * p_S_is_K / p_T_is_k);
}

		
static void Tests()
{
	printf("Tests: Partial Statelist sizes\n");
	for (uint16_t i = 0; i <= 16; i+=2) {
		printf("Partial State List Odd [%2d] has %8d entries\n", i, partial_statelist_odd[i].len);
	}
	for (uint16_t i = 0; i <= 16; i+=2) {
		printf("Partial State List Even	[%2d] has %8d entries\n", i, partial_statelist_even[i].len);
	}
	// printf("Tests: State List Odd [4] content:\n");
	// for (uint32_t i = 0; i < partial_statelist_odd[4].len; i++) {
		// printf("State_List_Odd[4][%d] = 0x%08x\n", i, partial_statelist_odd[4].states[i]);
	// }
	
 	#define NUM_STATISTICS 100000
	uint64_t statistics[257];
	uint32_t statistics_odd[17];
	uint32_t statistics_even[17];
	struct Crypto1State cs;
	time_t time1 = clock();

	for (uint16_t i = 0; i < 257; i++) {
		statistics[i] = 0;
	}
	for (uint16_t i = 0; i < 17; i++) {
		statistics_odd[i] = 0;
		statistics_even[i] = 0;
	}
	
	for (uint64_t i = 0; i < NUM_STATISTICS; i++) {
		cs.odd = (rand() & 0xfff) << 12 | (rand() & 0xfff);
		cs.even = (rand() & 0xfff) << 12 | (rand() & 0xfff);
		uint16_t sum_property = SumProperty(&cs);
		statistics[sum_property] += 1;
		sum_property=SumPropertyEven(cs.even);
		statistics_even[sum_property]++;
		sum_property=SumPropertyOdd(cs.odd);
		statistics_odd[sum_property]++;
		if (i%(NUM_STATISTICS/100) == 0) printf("."); 
	}
	
	printf("\nTests: Calculated %d Sum properties in %0.3f seconds (%0.0f calcs/second)\n", NUM_STATISTICS, ((float)clock() - time1)/CLOCKS_PER_SEC, NUM_STATISTICS/((float)clock() - time1)*CLOCKS_PER_SEC);
	for (uint16_t i = 0; i < 257; i++) {
		if (statistics[i] != 0) {
			printf("probability[%3d] = %0.5f\n", i, (float)statistics[i]/NUM_STATISTICS);
		}
	}
	for (uint16_t i = 0; i <= 16; i++) {
		if (statistics_odd[i] != 0) {
			printf("probability odd [%2d] = %0.5f\n", i, (float)statistics_odd[i]/NUM_STATISTICS);
		}
	}
	for (uint16_t i = 0; i <= 16; i++) {
		if (statistics_odd[i] != 0) {
			printf("probability even [%2d] = %0.5f\n", i, (float)statistics_even[i]/NUM_STATISTICS);
		}
	}

	printf("Tests: Sum Probabilities based on Partial Sums\n");
	for (uint16_t i = 0; i < 257; i++) {
		statistics[i] = 0;
	}
	uint64_t num_states = 0;
	for (uint16_t oddsum = 0; oddsum <= 16; oddsum += 2) {
		for (uint16_t evensum = 0; evensum <= 16; evensum += 2) {
			uint16_t sum = oddsum*(16-evensum) + (16-oddsum)*evensum;
			statistics[sum] += (uint64_t)partial_statelist_odd[oddsum].len * (1<<4) * partial_statelist_even[evensum].len * (1<<5);
			num_states += (uint64_t)partial_statelist_odd[oddsum].len * (1<<4) * partial_statelist_even[evensum].len * (1<<5);
		}
	}
	printf("num_states = %lld, expected %lld\n", num_states, (1LL<<48));
	for (uint16_t i = 0; i < 257; i++) {
		if (statistics[i] != 0) {
			printf("probability[%3d] = %0.5f\n", i, (float)statistics[i]/num_states);
		}
	}
	
	printf("\nTests: Hypergeometric Probability for selected parameters\n");
	printf("p_hypergeometric(256, 206, 255, 206) = %0.8f\n", p_hypergeometric(256, 206, 255, 206));
	printf("p_hypergeometric(256, 206, 255, 205) = %0.8f\n", p_hypergeometric(256, 206, 255, 205));
	printf("p_hypergeometric(256, 156, 1, 1) = %0.8f\n", p_hypergeometric(256, 156, 1, 1));
	printf("p_hypergeometric(256, 156, 1, 0) = %0.8f\n", p_hypergeometric(256, 156, 1, 0));
	printf("p_hypergeometric(256, 1, 1, 1) = %0.8f\n", p_hypergeometric(256, 1, 1, 1));
	printf("p_hypergeometric(256, 1, 1, 0) = %0.8f\n", p_hypergeometric(256, 1, 1, 0));
	
	struct Crypto1State *pcs;
	pcs = crypto1_create(0xffffffffffff);
	printf("\nTests: for key = 0xffffffffffff:\nSum(a0) = %d\nodd_state =  0x%06x\neven_state = 0x%06x\n", 
		SumProperty(pcs), pcs->odd & 0x00ffffff, pcs->even & 0x00ffffff);
	crypto1_destroy(pcs);
	pcs = crypto1_create(0xa0a1a2a3a4a5);
	printf("Tests: for key = 0xa0a1a2a3a4a5:\nSum(a0) = %d\nodd_state =  0x%06x\neven_state = 0x%06x\n",
		SumProperty(pcs), pcs->odd & 0x00ffffff, pcs->even & 0x00ffffff);
	crypto1_destroy(pcs);

	}
	

static float estimate_second_byte_sum(uint8_t *best_first_byte, uint16_t *best_Sum8_guess) 
{
	float max_prob = 0.0;
	for (uint16_t first_byte = 0; first_byte < 256; first_byte++) {
		float Sum8_prob = 0.0;
		uint16_t Sum8 = 0;
		if (nonces[first_byte].updated) {
			for (uint16_t sum = 0; sum <= 256; sum++) {
				float prob = sum_probability(sum, nonces[first_byte].num, nonces[first_byte].Sum);
				if (prob > Sum8_prob) {
					Sum8_prob = prob;
					Sum8 = sum;
				}
			}
			nonces[first_byte].Sum8_guess = Sum8;
			nonces[first_byte].Sum8_prob = Sum8_prob;
			nonces[first_byte].updated = false;
		}
		if (nonces[first_byte].Sum8_prob > max_prob) {
			max_prob = nonces[first_byte].Sum8_prob;
			*best_first_byte = first_byte;
			*best_Sum8_guess = nonces[first_byte].Sum8_guess;
		}
	}
	return max_prob;
}	


static int read_nonce_file(void)
{
	FILE *fnonces = NULL;
	uint8_t trgBlockNo;
	uint8_t trgKeyType;
	uint8_t read_buf[9];
	uint32_t nt_enc1, nt_enc2;
	uint8_t par_enc;
	int total_num_nonces = 0;
	
	if ((fnonces = fopen("nonces.bin","rb")) == NULL) { 
		PrintAndLog("Could not open file nonces.bin");
		return 1;
	}

	PrintAndLog("Reading nonces from file nonces.bin...");
	if (fread(read_buf, 1, 6, fnonces) == 0) {
		PrintAndLog("File reading error.");
		fclose(fnonces);
		return 1;
	}
	cuid = bytes_to_num(read_buf, 4);
	trgBlockNo = bytes_to_num(read_buf+4, 1);
	trgKeyType = bytes_to_num(read_buf+5, 1);

	while (fread(read_buf, 1, 9, fnonces) == 9) {
		nt_enc1 = bytes_to_num(read_buf, 4);
		nt_enc2 = bytes_to_num(read_buf+4, 4);
		par_enc = bytes_to_num(read_buf+8, 1);
		//printf("Encrypted nonce: %08x, encrypted_parity: %02x\n", nt_enc1, par_enc >> 4);
		//printf("Encrypted nonce: %08x, encrypted_parity: %02x\n", nt_enc2, par_enc & 0x0f);
		add_nonce(nt_enc1, par_enc >> 4);
		add_nonce(nt_enc2, par_enc & 0x0f);
		total_num_nonces += 2;
	}
	fclose(fnonces);
	PrintAndLog("Read %d nonces from file. cuid=%08x, Block=%d, Keytype=%c", total_num_nonces, cuid, trgBlockNo, trgKeyType==0?'A':'B');

	return 0;
}


int static acquire_nonces(uint8_t blockNo, uint8_t keyType, uint8_t *key, uint8_t trgBlockNo, uint8_t trgKeyType, bool nonce_file_write, bool slow)
{
	clock_t time1 = clock();
	bool initialize = true;
	bool field_off = false;
	bool finished = false;
	uint32_t flags = 0;
	uint8_t write_buf[9];
	uint32_t total_num_nonces = 0;
	uint32_t next_thousand = 1000;
	uint32_t total_added_nonces = 0;
	FILE *fnonces = NULL;
	UsbCommand resp;

	#define CONFIDENCE_THRESHOLD	0.95		// Collect nonces until we are certain enough to have guessed Sum(a8) correctly

	clearCommandBuffer();

	do {
		flags = 0;
		flags |= initialize ? 0x0001 : 0;
		flags |= slow ? 0x0002 : 0;
		flags |= field_off ? 0x0004 : 0;
		UsbCommand c = {CMD_MIFARE_ACQUIRE_ENCRYPTED_NONCES, {blockNo + keyType * 0x100, trgBlockNo + trgKeyType * 0x100, flags}};
		memcpy(c.d.asBytes, key, 6);

		SendCommand(&c);
		
		if (field_off) finished = true;
		
		if (initialize) {
			if (!WaitForResponseTimeout(CMD_ACK, &resp, 3000)) return 1;
			if (resp.arg[0]) return resp.arg[0];  // error during nested_hard

			cuid = resp.arg[1];
			// PrintAndLog("Acquiring nonces for CUID 0x%08x", cuid); 
			if (nonce_file_write && fnonces == NULL) {
				if ((fnonces = fopen("nonces.bin","wb")) == NULL) { 
					PrintAndLog("Could not create file nonces.bin");
					return 3;
				}
				PrintAndLog("Writing acquired nonces to binary file nonces.bin");
				num_to_bytes(cuid, 4, write_buf);
				fwrite(write_buf, 1, 4, fnonces);
				fwrite(&trgBlockNo, 1, 1, fnonces);
				fwrite(&trgKeyType, 1, 1, fnonces);
			}
		}

		if (!initialize) {
			uint32_t nt_enc1, nt_enc2;
			uint8_t par_enc;
			uint16_t num_acquired_nonces = resp.arg[2];
			uint8_t *bufp = resp.d.asBytes;
			for (uint16_t i = 0; i < num_acquired_nonces; i+=2) {
				nt_enc1 = bytes_to_num(bufp, 4);
				nt_enc2 = bytes_to_num(bufp+4, 4);
				par_enc = bytes_to_num(bufp+8, 1);
				
				//printf("Encrypted nonce: %08x, encrypted_parity: %02x\n", nt_enc1, par_enc >> 4);
				total_added_nonces += add_nonce(nt_enc1, par_enc >> 4);
				//printf("Encrypted nonce: %08x, encrypted_parity: %02x\n", nt_enc2, par_enc & 0x0f);
				total_added_nonces += add_nonce(nt_enc2, par_enc & 0x0f);
				

				if (nonce_file_write) {
					fwrite(bufp, 1, 9, fnonces);
				}
				
				bufp += 9;
			}

			total_num_nonces += num_acquired_nonces;
		}
		
		if (first_byte_num == 256 ) {
			// printf("first_byte_num = %d, first_byte_Sum = %d\n", first_byte_num, first_byte_Sum);
			float last_confidence = guessed_Sum8_confidence;
			uint16_t last_Sum8 = guessed_Sum8;
			guessed_Sum8_confidence = estimate_second_byte_sum(&best_first_byte, &guessed_Sum8);
			if (guessed_Sum8_confidence > last_confidence || guessed_Sum8 != last_Sum8 || total_num_nonces > next_thousand) {
				next_thousand = (total_num_nonces/1000+1) * 1000;
				PrintAndLog("Acquired %5d nonces (%5d with distinct bytes 0 and 1). Guessed Sum(a8) = %3d for first nonce byte = 0x%02x, probability for correct guess = %1.2f%%",
					total_num_nonces, 
					total_added_nonces,
					guessed_Sum8, 
					best_first_byte, 
					guessed_Sum8_confidence*100);
			}
			if (guessed_Sum8_confidence >= CONFIDENCE_THRESHOLD) {
				field_off = true;	// switch off field with next SendCommand and then finish
			}
		}

		if (!initialize) {
			if (!WaitForResponseTimeout(CMD_ACK, &resp, 3000)) return 1;
			if (resp.arg[0]) return resp.arg[0];  // error during nested_hard
		}

		initialize = false;

	} while (!finished);

	
	if (nonce_file_write) {
		fclose(fnonces);
	}
	
	PrintAndLog("Acquired a total of %d nonces in %1.1f seconds (%d nonces/minute)", 
		total_num_nonces, 
		((float)clock()-time1)/CLOCKS_PER_SEC, 
		total_num_nonces*60*CLOCKS_PER_SEC/(clock()-time1));
	
	return 0;
}


static int init_partial_statelists(void)
{
	printf("Allocating memory for partial statelists...\n");
	for (uint16_t i = 0; i <= 16; i++) {
		partial_statelist_odd[i].len = 0;
		if (i % 2) {	// partial Sum Properties are even.
			partial_statelist_odd[i].states = NULL;
		} else {	
			// 20 Bits are relevant for odd states. Less than a half per Sum is expected
			partial_statelist_odd[i].states = malloc(sizeof(uint32_t) << 19);  
			if (partial_statelist_odd[i].states == NULL) {
				PrintAndLog("Cannot allocate enough memory. Aborting");
				return 4;
			}
			for (uint16_t j = 0; j < 256; j++) {
				partial_statelist_odd[i].index[j] = NULL;
			}
		}
		partial_statelist_even[i].len = 0;
		if (i % 2) {	// partial Sum Properties are even.
			partial_statelist_even[i].states = NULL;
		} else {
			// 19 Bits are relevant for even states. Less than a half per Sum is expected
			partial_statelist_even[i].states = malloc(sizeof(uint32_t) << 18);
			if (partial_statelist_even[i].states == NULL) {
				PrintAndLog("Cannot allocate enough memory. Aborting");
				return 4;
			}
			for (uint16_t j = 0; j < 256; j++) {
				partial_statelist_even[i].index[j] = NULL;
			}
		}
	}
		
	printf("Generating partial statelists odd...\n");
	uint32_t index = -1;
	for (uint32_t oddstate = 0; oddstate < (1 << 20); oddstate++) {
		uint16_t odd_sum_property = SumPropertyOdd(oddstate);
		uint32_t *p = partial_statelist_odd[odd_sum_property].states;
		p += partial_statelist_odd[odd_sum_property].len;
		*p = oddstate;
		partial_statelist_odd[odd_sum_property].len++;
		if ((oddstate & 0x000ff000) != index) {
			index = oddstate & 0x000ff000;
		}
		if (partial_statelist_odd[odd_sum_property].index[index >> 12] == NULL) {
			partial_statelist_odd[odd_sum_property].index[index >> 12] = p;
		}
	}
	// add End Of List markers
	for (uint16_t i = 0; i <= 16; i += 2) {
		uint32_t *p = partial_statelist_odd[i].states;
		p += partial_statelist_odd[i].len;
		*p = 0xffffffff;
	}
	
	printf("Generating partial statelists even...\n");
	index = -1;
	for (uint32_t evenstate = 0; evenstate < (1 << 19); evenstate++) {
		uint16_t even_sum_property = SumPropertyEven(evenstate);
		uint32_t *p = partial_statelist_even[even_sum_property].states;
		p += partial_statelist_even[even_sum_property].len;
		*p = evenstate;
		partial_statelist_even[even_sum_property].len++;
		if ((evenstate & 0x000ff000) != index) {
			index = evenstate & 0x000ff000;
		}
		if (partial_statelist_even[even_sum_property].index[index >> 12] == NULL) {
			partial_statelist_even[even_sum_property].index[index >> 12] = p;
		}
	}
	// add End Of List markers
	for (uint16_t i = 0; i <= 16; i += 2) {
		uint32_t *p = partial_statelist_even[i].states;
		p += partial_statelist_even[i].len;
		*p = 0xffffffff;
	}
	
	return 0;
}	
		
		
static void add_state(statelist_t *sl, uint32_t state, odd_even_t odd_even)
{
	uint32_t *p;

	p = sl->states[odd_even];
	p += sl->len[odd_even];
	*p = state;
	sl->len[odd_even]++;
}


uint32_t *find_first_state(uint32_t state, partial_indexed_statelist_t *sl)
{
	uint32_t *p = sl->index[state >> 12];		// first 8 Bits as index

	if (p == NULL) return NULL;
	while ((*p & 0x000ffff0) < state) p++;
	if (*p == 0xffffffff) return NULL;			// reached end of list, no match
	if ((*p & 0x000ffff0) == state) return p;	// found a match.
	return NULL;								// no match
}


static int add_matching_states(statelist_t *candidates, uint16_t part_sum_a0, uint16_t part_sum_a8, odd_even_t odd_even)
{
	uint32_t worstcase_size = (odd_even==ODD_STATE) ? 1<<24 : 1<<23;
	
	if (odd_even == ODD_STATE) {
		candidates->states[odd_even] = (uint32_t *)malloc(sizeof(uint32_t) * worstcase_size);
		if (candidates->states[odd_even] == NULL) {
			PrintAndLog("Out of memory error.\n");
			return 4;
		}
		for (uint32_t *p1 = partial_statelist_odd[part_sum_a0].states; *p1 != 0xffffffff; p1++) {
			uint32_t *p2 = find_first_state((*p1 << 4) & 0x000ffff0, &partial_statelist_odd[part_sum_a8]);
			while (p2 != NULL && ((*p1 << 4) & 0x000ffff0) == (*p2 & 0x000ffff0) && *p2 != 0xffffffff) {
				add_state(candidates, (*p1 << 4) | *p2, odd_even);
				p2++;
			}
			p2 = candidates->states[odd_even];
			p2 += candidates->len[odd_even];
			*p2 = 0xffffffff;
		}
		candidates->states[odd_even] = realloc(candidates->states[odd_even], sizeof(uint32_t) * (candidates->len[odd_even] + 1));
	} else {
		candidates->states[odd_even] = (uint32_t *)malloc(sizeof(uint32_t) * worstcase_size);
		if (candidates->states[odd_even] == NULL) {
			PrintAndLog("Out of memory error.\n");
			return 4;
		}
		for (uint32_t *p1 = partial_statelist_even[part_sum_a0].states; *p1 != 0xffffffff; p1++) {
			uint32_t *p2 = find_first_state((*p1 << 4) & 0x0007fff0, &partial_statelist_even[part_sum_a8]);
			while (p2 != NULL && ((*p1 << 4) & 0x0007fff0) == (*p2 & 0x0007fff0) && *p2 != 0xffffffff) {
				add_state(candidates, (*p1 << 4) | *p2, odd_even);
				p2++;
			}
			p2 = candidates->states[odd_even];
			p2 += candidates->len[odd_even];
			*p2 = 0xffffffff;
		}
		candidates->states[odd_even] = realloc(candidates->states[odd_even], sizeof(uint32_t) * (candidates->len[odd_even] + 1));
	}
	return 0;
}


static statelist_t *add_more_candidates(statelist_t *current_candidates)
{
	statelist_t *new_candidates = NULL;
	if (current_candidates == NULL) {
		if (candidates == NULL) {
			candidates = (statelist_t *)malloc(sizeof(statelist_t));
		}
		new_candidates = candidates;
	} else {
		new_candidates = current_candidates->next = (statelist_t *)malloc(sizeof(statelist_t));
	}
	new_candidates->next = NULL;
	new_candidates->len[ODD_STATE] = 0;
	new_candidates->len[EVEN_STATE] = 0;
	new_candidates->states[ODD_STATE] = NULL;
	new_candidates->states[EVEN_STATE] = NULL;
	return new_candidates;
}


static void TestIfKeyExists(uint64_t key)
{
	struct Crypto1State *pcs;
	pcs = crypto1_create(key);
	crypto1_byte(pcs, (cuid >> 24) ^ best_first_byte, true);

	uint32_t state_odd = pcs->odd & 0x00ffffff;
	uint32_t state_even = pcs->even & 0x00ffffff;
	printf("searching for key %llx after first byte 0x%02x (state_odd = 0x%06x, state_even = 0x%06x) ...\n", key, best_first_byte, state_odd, state_even);
	
	for (statelist_t *p = candidates; p != NULL; p = p->next) {
		uint32_t *p_odd = p->states[ODD_STATE];
		uint32_t *p_even = p->states[EVEN_STATE];
		while (*p_odd != 0xffffffff) {
			if (*p_odd == state_odd) printf("o");
			p_odd++;
		}
		while (*p_even != 0xffffffff) {
			if (*p_even == (state_even & 0x007fffff)) printf("e");
			p_even++;
		}
		printf("|");
	}
	printf("\n");
	crypto1_destroy(pcs);
}

	
static void generate_candidates(uint16_t sum_a0, uint16_t sum_a8)
{
	printf("Generating crypto1 state candidates... \n");
	
	statelist_t *current_candidates = NULL;
	// estimate maximum candidate states
	uint64_t maximum_states = 0;
	for (uint16_t sum_odd = 0; sum_odd <= 16; sum_odd += 2) {
		for (uint16_t sum_even = 0; sum_even <= 16; sum_even += 2) {
			if (sum_odd*(16-sum_even) + (16-sum_odd)*sum_even == sum_a0) {
				maximum_states += (uint64_t)partial_statelist_odd[sum_odd].len * (1<<4) * partial_statelist_even[sum_even].len * (1<<5);
			}
		}
	}
	printf("Estimated number of possible keys with S(a0) = %d: %lld (2^%1.1f)\n", sum_a0, maximum_states, log(maximum_states)/log(2.0));
	
	for (uint16_t p = 0; p <= 16; p += 2) {
		for (uint16_t q = 0; q <= 16; q += 2) {
			if (p*(16-q) + (16-p)*q == sum_a0) {
				printf("Reducing Partial Statelists (p,q) = (%d,%d) with lengths %d, %d\n", 
						p, q, partial_statelist_odd[p].len, partial_statelist_even[q].len);
				for (uint16_t r = 0; r <= 16; r += 2) {
					for (uint16_t s = 0; s <= 16; s += 2) {
						if (r*(16-s) + (16-r)*s == sum_a8) {
							current_candidates = add_more_candidates(current_candidates);
							add_matching_states(current_candidates, p, r, ODD_STATE);
							printf("Odd state candidates: %d (2^%0.1f)\n", current_candidates->len[ODD_STATE], log(current_candidates->len[ODD_STATE])/log(2)); 
							add_matching_states(current_candidates, q, s, EVEN_STATE);
							printf("Even state candidates: %d (2^%0.1f)\n", current_candidates->len[EVEN_STATE]*2, log(current_candidates->len[EVEN_STATE]*2)/log(2)); 
						}
					}
				}
			}
		}
	}					

	
	maximum_states = 0;
	for (statelist_t *sl = candidates; sl != NULL; sl = sl->next) {
		maximum_states += (uint64_t)sl->len[ODD_STATE] * sl->len[EVEN_STATE] * 2;
	}
	printf("Estimated number of possible keys with S(a0) = %d AND S(a8)=%d: %lld (2^%1.1f)\n", sum_a0, sum_a8, maximum_states, log(maximum_states)/log(2.0));

	TestIfKeyExists(0xffffffffffff);
	TestIfKeyExists(0xa0a1a2a3a4a5);
	
}


int mfnestedhard(uint8_t blockNo, uint8_t keyType, uint8_t *key, uint8_t trgBlockNo, uint8_t trgKeyType, bool nonce_file_read, bool nonce_file_write, bool slow) 
{
	
	// initialize the list of nonces
	for (uint16_t i = 0; i < 256; i++) {
		nonces[i].num = 0;
		nonces[i].Sum = 0;
		nonces[i].Sum8_guess = 0;
		nonces[i].Sum8_prob = 0.0;
		nonces[i].updated = true;
		nonces[i].first = NULL;
	}
	first_byte_num = 0;
	first_byte_Sum = 0;
	guessed_Sum8 = 0;
	best_first_byte = 0;
	guessed_Sum8_confidence = 0.0;
		
	init_partial_statelists();
	
	if (nonce_file_read) {  	// use pre-acquired data from file nonces.bin
		if (read_nonce_file() != 0) {
			return 3;
		}
		guessed_Sum8_confidence = estimate_second_byte_sum(&best_first_byte, &guessed_Sum8);
	} else {					// acquire nonces.
		uint16_t is_OK = acquire_nonces(blockNo, keyType, key, trgBlockNo, trgKeyType, nonce_file_write, slow);
		if (is_OK != 0) {
			return is_OK;
		}
	}

	Tests();

	PrintAndLog("");
	PrintAndLog("Sum(a0) = %d", first_byte_Sum);
	PrintAndLog("Guess for Sum(a8) = %d for first nonce byte = 0x%02x, n = %d, k = %d, probability for correct guess = %1.0f%%\n", 
		guessed_Sum8,
		best_first_byte,
		nonces[best_first_byte].num,
		nonces[best_first_byte].Sum,
		guessed_Sum8_confidence*100);		

	generate_candidates(first_byte_Sum, guessed_Sum8);
	
	PrintAndLog("Brute force phase not yet implemented");
	
	return 0;
}


