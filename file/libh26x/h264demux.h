#ifndef __H264DEMUX__H__
#define __H264DEMUX__H__

#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>    

typedef struct {
    int timescale;
    int framerate;
    int width;
    int height;
    const char* sps;
    int spslen;
    const char* pps;
    int ppslen;    
    int headerlen;
}H264Configuration_t;

void* H264Demux_Init(const char* filepath, int circleread);

int H264Demux_GetConfig(void *handle, H264Configuration_t *config);

int H264Demux_GetFrame(void* handle, const char **frame, int *length);

int H264Demux_CLose(void *handle);

#endif