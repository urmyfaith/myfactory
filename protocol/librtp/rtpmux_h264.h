#ifndef __RTPMUX_H264_h__
#define __RTPMUX_H264_h__

#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>    

#include "rtp.h"

void* rtpmux_h264_alloc(unsigned long ssrc);

int rtpmux_h264_setframe(void* handle, const char* frame_buffer, int frame_length);

int rtpmux_h264_getpacket(void* handle, const char **rtp_buffer, int *rtp_packet_length,
            int *rtp_last_packet_length, int *rtp_packet_count);

int rtpmux_h264_free(void* handle);

#endif