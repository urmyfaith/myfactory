#ifndef __SDP_DEMUX_H__
#define __SDP_DEMUX_H__

#include <stdint.h>

struct sdp_demux_header
{
	char* headername;
	char* headervalue;
};

int sdp_demux_alloc(void** inst);

int sdp_demux(void* inst, char* content, int contentlength, sdp_demux_header** headers, int *headercount);

int sdp_demux_free(void* inst);

#endif