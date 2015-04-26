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
#include "zlib.h"

#define MAX(a,b) ((a)>(b)?(a):(b))

// zlib configuration
#define COMPRESS_LEVEL			9		// use best possible compression

#define FPGA_CONFIG_SIZE	42175
static uint8_t fpga_config[FPGA_CONFIG_SIZE];

static void usage(char *argv0)
{
	fprintf(stderr, "Usage:   %s <infile> <outfile>\n\n", argv0);
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


int zlib_compress(FILE *infile, FILE *outfile)
{
	int i, ret;
	z_stream compressed_fpga_stream;
	
	// read the input file into fpga_config[] and count occurrences of each symbol:
	i = 0;
	while(!feof(infile)) {
		uint8_t c;
		c = fgetc(infile);
		fpga_config[i++] = c;
		if (i > FPGA_CONFIG_SIZE+1) {
			fprintf(stderr, "Input file too big (> %d bytes). This is probably not a PM3 FPGA config file.", FPGA_CONFIG_SIZE);
			fclose(infile);
			fclose(outfile);
			return -1;
		}
	}

	// initialize zlib structures
	compressed_fpga_stream.next_in = fpga_config;
	compressed_fpga_stream.avail_in = i;
	compressed_fpga_stream.zalloc = fpga_deflate_malloc;
	compressed_fpga_stream.zfree = fpga_deflate_free;
	
	// estimate the size of the compressed output
	unsigned int outsize_max = deflateBound(&compressed_fpga_stream, compressed_fpga_stream.avail_in);
	uint8_t *outbuf = malloc(outsize_max);
	compressed_fpga_stream.next_out = outbuf;
	compressed_fpga_stream.avail_out = outsize_max;
	fprintf(stderr, "Allocated %d bytes for output file (estimated upper bound)\n", outsize_max);
	
	ret = deflateInit(&compressed_fpga_stream, COMPRESS_LEVEL);
				
	if (ret == Z_OK) {
		ret = deflate(&compressed_fpga_stream, Z_FINISH);
	}
	
	fprintf(stderr, "produced %d bytes of output\n", compressed_fpga_stream.total_out);

	if (ret != Z_STREAM_END) {
		fprintf(stderr, "Error in deflate(): %d %s\n", ret, compressed_fpga_stream.msg);
		free(outbuf);
		deflateEnd(&compressed_fpga_stream);
		fclose(infile);
		fclose(outfile);
		return -1;
		}
		
	for (i = 0; i < compressed_fpga_stream.total_out; i++) {
		fputc(outbuf[i], outfile);
	}	

	free(outbuf);
	deflateEnd(&compressed_fpga_stream);
	fclose(infile);
	fclose(outfile);

	return 0;
	
}



int main(int argc, char **argv)
{
	char *infilename;
	char *outfilename;
	
	if (argc != 3) {
		usage(argv[0]);
		return -1;
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
		
	return zlib_compress(infile, outfile);
}
