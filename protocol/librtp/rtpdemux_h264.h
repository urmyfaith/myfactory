#ifndef __RTPDEMUX_H264_h__
#define __RTPDEMUX_H264_h__

#include <stdio.h>  
#include <stdlib.h>  
#include <stdint.h>  
#include <string.h>    

#include "rtp.h"

typedef int (*rtpdemux_h264frame_callback)(const char* frame, int framelength, uint32_t timestamp, uint32_t ssrc);

void* rtpdemux_h264_alloc(rtpdemux_h264frame_callback callback);

int rtpdemux_h264_setpacket(void* handle, char* packet_buffer, int packet_length);

int rtpdemux_h264_free(void* handle);

#endif