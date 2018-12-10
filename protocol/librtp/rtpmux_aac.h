#ifndef __RTPMUX_AAC_h__
#define __RTPMUX_AAC_h__

#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>    

#include "rtp.h"

void* rtpmux_aac_alloc(unsigned long ssrc);

int rtpmux_aac_setframe(void* handle, const char* frame_buffer, int frame_length);

int rtpmux_aac_getpacket(void* handle, const char **rtp_buffer, int *rtp_packet_length,
            int *rtp_last_packet_length, int *rtp_packet_count);

int rtpmux_aac_free(void* handle);

#endif