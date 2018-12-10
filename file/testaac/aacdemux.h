#ifndef __ADTSDEMUX__H__
#define __ADTSDEMUX__H__

#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>    
#include <stdint.h>

typedef struct {
    uint32_t samplerate;
    uint32_t samplerateindex;
    uint32_t profilelevel;
    uint32_t channelcount;
    int headerlen;
}AACConfiguration_t;

void* AACDemux_Init(const char* filepath, int circleread);

int AACDemux_GetConfig(void *handle, AACConfiguration_t *config);

int AACDemux_GetFrame(void* handle, const char **frame, int *length);

int AACDemux_CLose(void *handle);

#endif