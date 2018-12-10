#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#include "libmp4/mp4_demux.h"

int test1(char* filepath)
{
	struct mp4_demux *demux = mp4_demux_open2(filepath);
	if (demux == NULL) {
		fprintf(stderr, "mp4_demux_open() failed\n");
		return -1;
	}

	uint32_t retBytes = mp4_demux_listboxes(demux);
	if (retBytes < 0) {
		fprintf(stderr, "retBytes < 0 \n");
	}


	int ret = mp4_demux_close(demux);
	if (ret < 0) {
		fprintf(stderr, "mp4_demux_close() failed\n");
		return  -1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s <file>\n", argv[0]);
		exit(-1);
	}

	return test1(argv[1]);
}