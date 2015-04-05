//-----------------------------------------------------------------------------
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Flasher frontend tool
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sleep.h"
#include "proxmark3.h"
#include "flash.h"
#include "uart.h"
#include "usb_cmd.h"

#define MAX(a,b) ((a)>(b)?(a):(b))

struct huffman_record {
		int16_t symbol;
		uint16_t count;
		uint8_t code_size;
		uint8_t code;
		struct huffman_record *left;
		struct huffman_record *right;
		struct huffman_record *next;
	};

typedef struct huffman_record huffman_record_t;

#define FPGA_CONFIG_SIZE	42175
static uint8_t fpga_config[FPGA_CONFIG_SIZE];
static huffman_record_t leaf_nodes[256];
static uint8_t start_code[256];

static void usage(char *argv0)
{
	fprintf(stderr, "Usage:   %s [-d] <infile> <outfile>\n\n", argv0);
	fprintf(stderr, "\t-d\tdecompress\n\n");
}


void add_to_heap(huffman_record_t **heap, huffman_record_t *new_record)
{
	huffman_record_t *succ = *heap;
	huffman_record_t *pred = NULL;
	
//	fprintf(stderr, "Adding symbol %d, count %d\n", new_record->symbol, new_record->count);
	
	while (succ != NULL && new_record->count > succ->count) {
		pred = succ;
		succ = succ->next;
	}

	// insert new record
	new_record->next = succ;
	if (pred == NULL) {			// first record in heap
		*heap = new_record;
	} else {
		pred->next = new_record;
	}
}
	

uint16_t set_codesize(huffman_record_t *tree_ptr, uint8_t depth)
{
	uint16_t max_size = depth;
	tree_ptr->code_size = depth;
	if (tree_ptr->left != NULL) {
		max_size = MAX(set_codesize(tree_ptr->left, depth+1), max_size);
	}
	if (tree_ptr->right != NULL) {
		max_size = MAX(set_codesize(tree_ptr->right, depth+1), max_size);
	}
	return max_size;
}	

int huffman_encode(FILE *infile, FILE *outfile)
{
	int i;
	
	// init leaf_nodes:
	for (i = 0; i < 256; i++) {
		leaf_nodes[i].count = 0;
		leaf_nodes[i].symbol = i;
		leaf_nodes[i].left = NULL;
		leaf_nodes[i].right = NULL;
		leaf_nodes[i].next = NULL;
	}
	
	// read the input file into fpga_config[] and count occurrences of each symbol:
	i = 0;
	while(!feof(infile)) {
		uint8_t c;
		c = fgetc(infile);
		fpga_config[i++] = c;
		leaf_nodes[c].count++;
		if (i > FPGA_CONFIG_SIZE+1) {
			fprintf(stderr, "Input file too big (> %d bytes). This is probably not a PM3 FPGA config file.", FPGA_CONFIG_SIZE);
			fclose(infile);
			fclose(outfile);
			return -1;
		}
	}
	
	fprintf(stderr, "\nStatistics: (symbol: count)\n");
	for (i = 0; i < 256; i++) {
		fprintf(stderr, "%3d: %5d\n", i, leaf_nodes[i].count);
	}

	// build the Huffman tree:
	huffman_record_t *heap_ptr = NULL;

	for (i = 0; i < 256; i++) {
		add_to_heap(&heap_ptr, &leaf_nodes[i]); 
	}

	fprintf(stderr, "\nSorted statistics: (symbol: count)\n");
	for (huffman_record_t *p = heap_ptr; p != NULL; p = p->next) {
		fprintf(stderr, "%3d: %5d\n", p->symbol, p->count);
	}

	for (i = 0; i < 255; i++) {
		// remove and combine the first two nodes
		huffman_record_t *p1, *p2;
		p1 = heap_ptr;
		p2 = heap_ptr->next;
		heap_ptr = p2->next;
		huffman_record_t *new_node = malloc(sizeof(huffman_record_t));
		new_node->left = p1;
		new_node->right = p2;
		new_node->count = p1->count + p2->count;
		add_to_heap(&heap_ptr, new_node); 
	}
	
	uint16_t max_codesize = set_codesize(heap_ptr, 0);

	fprintf(stderr, "\nStatistics: (symbol: count, codesize)\n");
	uint32_t compressed_size = 0;
	for (i = 0; i < 256; i++) {
		fprintf(stderr, "%3d: %5d, %d\n", leaf_nodes[i].symbol, leaf_nodes[i].count, leaf_nodes[i].code_size);
		compressed_size += leaf_nodes[i].count * leaf_nodes[i].code_size;
	}
	fprintf(stderr, "Compressed size = %ld (%f% of original size)", (compressed_size+7)/8, (float)(compressed_size)/(FPGA_CONFIG_SIZE * 8) * 100);
	fprintf(stderr, "Max Codesize = %d bits", max_codesize);
	
	uint8_t code = 0;
	for (i = max_codesize; i > 0; i--) {
		code = (code + 1) >> 1;
		start_code[i] = code;
		for (uint16_t j = 0; j < 256; j++) {
			if (leaf_nodes[j].code_size == i) {
				leaf_nodes[j].code = code;
				code++;
			}
		}
	}

	
	fprintf(stderr, "\nStatistics: (symbol: count, codesize, code)\n");
	for (i = 0; i < 256; i++) {
		fprintf(stderr, "%3d: %5d, %d, %02x\n", leaf_nodes[i].symbol, leaf_nodes[i].count, leaf_nodes[i].code_size, leaf_nodes[i].code);
	}
		
	fclose(infile);
	fclose(outfile);
	
	return 0;
}

int huffman_decode(FILE *infile, FILE *outfile)
{
	return 0;
}


int main(int argc, char **argv)
{
	bool decode = false;
	char *infilename;
	char *outfilename;
	
	if (argc < 3) {
		usage(argv[0]);
		return -1;
	}

	if (argc > 3) {
		if (!strcmp(argv[1], "-d")) {
			decode = true;
			infilename = argv[2];
			outfilename = argv[3];
		} else {
			usage(argv[0]);
			return -1;
		}
	} else {
		infilename = argv[1];
		outfilename = argv[2];
	}	

	FILE *infile = fopen(infilename, "rb");
	if (infile == NULL) {
		fprintf(stderr, "Error. Cannot open input file %s", infilename);
		return -1;
		}
		
	FILE *outfile = fopen(outfilename, "wb");
	if (outfile == NULL) {
		fprintf(stderr, "Error. Cannot open output file %s", outfilename);
		fclose(infile);
		return -1;
		}
		
	if (decode) {
		return huffman_decode(infile, outfile);
	} else {
		return huffman_encode(infile, outfile);
	}
}
