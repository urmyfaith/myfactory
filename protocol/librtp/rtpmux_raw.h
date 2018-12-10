#ifndef __RTPMUX_RAW_h__
#define __RTPMUX_RAW_h__

#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>    

#include "rtp.h"

void* rtpmux_raw_alloc(unsigned long ssrc);

int rtpmux_raw_setframe(void* handle, int streamtype, const char* frame_buffer, int frame_length);

int rtpmux_raw_getpacket(void* handle, const char **rtp_buffer, int *rtp_packet_length,
            int *rtp_last_packet_length, int *rtp_packet_count);

int rtpmux_raw_free(void* handle);

#endif