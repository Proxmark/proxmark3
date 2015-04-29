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
#include <stdint.h>
#include <stdbool.h>
#include "zlib.h"

#define MAX(a,b) ((a)>(b)?(a):(b))

// zlib configuration
#define COMPRESS_LEVEL			9		// use best possible compression
#define COMPRESS_WINDOW_BITS	15		// default = 15 for a window of 2^15 = 32KBytes
#define COMPRESS_MEM_LEVEL		9		// determines the amount of memory allocated during compression. Default = 8. Must be < 9
/* COMPRESS_STRATEGY can be 
	Z_DEFAULT_STRATEGY (the default), 
	Z_FILTERED (more huffmann, less string matching),
	Z_HUFFMAN_ONLY (huffman only, no string matching)
	Z_RLE (distances limited to one)
	Z_FIXED (prevents the use of dynamic Huffman codes)
*/	
#define	COMPRESS_STRATEGY		Z_DEFAULT_STRATEGY
// zlib tuning parameters:
#define COMPRESS_GOOD_LENGTH		258
#define	COMPRESS_MAX_LAZY			258	
#define	COMPRESS_MAX_NICE_LENGTH	258
#define	COMPRESS_MAX_CHAIN			8192

#define FPGA_INTERLEAVE_SIZE	288 	// (the FPGA's internal config frame size is 288 bits. Interleaving with 288 bytes should give best compression)
#define FPGA_CONFIG_SIZE		42336	// our current fpga_[lh]f.bit files are 42175 bytes. Rounded up to next multiple of FPGA_INTERLEAVE_SIZE

static void usage(char *argv0)
{
	fprintf(stderr, "Usage:   %s <infile1> <infile2> ... <infile_n> <outfile>\n\n", argv0);
	fprintf(stderr, "Combines n FPGA bitstream files and compresses them into one.\n\n");
}


static voidpf fpga_deflate_malloc(voidpf opaque, uInt items, uInt size)
{
	fprintf(stderr, "zlib requested %d bytes\n", items*size);
	return malloc(items*size);
}


static void fpga_deflate_free(voidpf opaque, voidpf address)
{
	fprintf(stderr, "zlib frees memory\n");
	return free(address);
}


static bool all_feof(FILE *infile[], uint8_t num_infiles)
{
	for (uint16_t i = 0; i < num_infiles; i++) {
		if (!feof(infile[i])) {
			return false;
		}
	}
	
	return true;
}


int zlib_compress(FILE *infile[], uint8_t num_infiles, FILE *outfile)
{
	uint8_t *fpga_config;
	uint32_t i;
	int ret;
	uint8_t c;		
	z_stream compressed_fpga_stream;

	fpga_config = malloc(num_infiles * FPGA_CONFIG_SIZE);
	
	// read the input files interleaving into fpga_config[]
	i = 0;
	do {
		for(uint16_t j = 0; j < num_infiles; j++) {
			for(uint16_t k = 0; k < FPGA_INTERLEAVE_SIZE; k++) {
				c = fgetc(infile[j]);
				if (!feof(infile[j])) fpga_config[i++] = c; else fpga_config[i++] = '\0';
			}
		}

		if (i > num_infiles * FPGA_CONFIG_SIZE) {
			fprintf(stderr, "Input files too big (total of %ld > %d bytes). These are probably not PM3 FPGA config files.", i, num_infiles*FPGA_CONFIG_SIZE);
			for(uint16_t j = 0; j < num_infiles; j++) {
				fclose(infile[j]);
			}
			return -1;
		}
	} while (!all_feof(infile, num_infiles));

	fprintf(stderr, "Read a total of %ld bytes from %d files\n", i, num_infiles);
	
	// initialize zlib structures
	compressed_fpga_stream.next_in = fpga_config;
	compressed_fpga_stream.avail_in = i;
	compressed_fpga_stream.zalloc = fpga_deflate_malloc;
	compressed_fpga_stream.zfree = fpga_deflate_free;
	
	ret = deflateInit2(&compressed_fpga_stream, 
						COMPRESS_LEVEL,
						Z_DEFLATED,
						COMPRESS_WINDOW_BITS,
						COMPRESS_MEM_LEVEL,
						COMPRESS_STRATEGY);

	// estimate the size of the compressed output
	unsigned int outsize_max = deflateBound(&compressed_fpga_stream, compressed_fpga_stream.avail_in);
	fprintf(stderr, "Allocating %ld bytes for output file (estimated upper bound)\n", outsize_max);
	uint8_t *outbuf = malloc(outsize_max);
	compressed_fpga_stream.next_out = outbuf;
	compressed_fpga_stream.avail_out = outsize_max;
	
					
	if (ret == Z_OK) {
		ret = deflateTune(&compressed_fpga_stream,
							COMPRESS_GOOD_LENGTH,
							COMPRESS_MAX_LAZY,
							COMPRESS_MAX_NICE_LENGTH,
							COMPRESS_MAX_CHAIN);
	}
	
	if (ret == Z_OK) {
		ret = deflate(&compressed_fpga_stream, Z_FINISH);
	}
	
	fprintf(stderr, "produced %d bytes of output\n", compressed_fpga_stream.total_out);

	if (ret != Z_STREAM_END) {
		fprintf(stderr, "Error in deflate(): %d %s\n", ret, compressed_fpga_stream.msg);
		free(outbuf);
		deflateEnd(&compressed_fpga_stream);
		for(uint16_t j = 0; j < num_infiles; j++) {
			fclose(infile[j]);
		}
		fclose(outfile);
		free(infile);
		free(fpga_config);
		return -1;
		}
		
	for (i = 0; i < compressed_fpga_stream.total_out; i++) {
		fputc(outbuf[i], outfile);
	}	

	free(outbuf);
	deflateEnd(&compressed_fpga_stream);
	for(uint16_t j = 0; j < num_infiles; j++) {
		fclose(infile[j]);
	}
	fclose(outfile);
	free(infile);
	free(fpga_config);
	
	return 0;
	
}



int main(int argc, char **argv)
{
	FILE **infiles;
	FILE *outfile;
	
	if (argc == 1 || argc == 2) {
		usage(argv[0]);
		return -1;
	}
	
	infiles = calloc(argc-2, sizeof(FILE*));
	
	for (uint16_t i = 0; i < argc-2; i++) { 
		infiles[i] = fopen(argv[i+1], "rb");
		if (infiles[i] == NULL) {
			fprintf(stderr, "Error. Cannot open input file %s", argv[i+1]);
			return -1;
		}
	}

	outfile = fopen(argv[argc-1], "wb");
	if (outfile == NULL) {
		fprintf(stderr, "Error. Cannot open output file %s", argv[argc-1]);
		return -1;
		}
		
	return zlib_compress(infiles, argc-2, outfile);
}
