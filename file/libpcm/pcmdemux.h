#ifndef __PCMDEMUX__H__
#define __PCMDEMUX__H__

#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>    

typedef struct {
    uint32_t samplerate;
    uint32_t samplerateindex;
    uint32_t profilelevel;
    uint32_t channelcount;
    uint32_t bitcount;    
}PCMConfiguration_t;

enum PCMType
{
	PCM_G711U = 0,
	PCM_G711A = 8,
	PCM_G729 = 18
};

void* PCMDemux_Init(const char* filepath, uint32_t bitcount, uint32_t channelcount,
	uint32_t samplerate, PCMType pcmtype, int circleread);

int PCMDemux_GetConfig(void *handle, PCMConfiguration_t *config);

int PCMDemux_GetFrame(void* handle, const char **frame, int *length);

int PCMDemux_CLose(void *handle);

#endif