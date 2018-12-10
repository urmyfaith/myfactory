#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <errno.h>
#include <fstream>
#include <string>

#include "pcmdemux.h"  

typedef struct {
    PCMConfiguration_t config;

    std::string frame;
    std::string databuffer;

    char buffer[1024 * 1024];
    std::fstream fs;

    int circleread;
    uint32_t defaultframelength;
    PCMType pcmType;
}PCMFileDesc_t;

static int pcm_sample_rates[16] =
{
  96000, 88200, 64000, 48000, 44100, 32000,
  24000, 22050, 16000, 12000, 11025, 8000, 7350,
  0, 0, 0
};

int PCMReadOneFrameFromBuf(void* handle)
{
    PCMFileDesc_t *desc = (PCMFileDesc_t*)handle;

    if( desc->databuffer.size() < desc->defaultframelength )
    {
        printf("read from databuffer error \n");
        return -1;
    }

    desc->frame.clear();
    desc->frame.append(desc->databuffer.data(), desc->defaultframelength);
    desc->databuffer.erase(0, desc->defaultframelength);

    return desc->defaultframelength;
}

int PCMReadData(void* handle)
{
    PCMFileDesc_t *desc = (PCMFileDesc_t*)handle;

    desc->fs.read(desc->buffer, sizeof(desc->buffer));
    int length = desc->fs.gcount();
    if( length <= 0 )
    {
        printf("seek file to the beginning \n");
        if( desc->circleread == 0 )
            return -1;
        
        desc->fs.clear();
        desc->fs.seekg(0, std::ios::beg);
 
         desc->fs.read(desc->buffer, sizeof(desc->buffer));    
        length = desc->fs.gcount();
        if( length <= 0 )
        {
            printf("file read error \n");
            return -1;
        }
    }

    desc->databuffer.append(desc->buffer, length);
    return 0;
}

int _PCMGetFrame(void* handle, const char **frame, int *length)  
{
    PCMFileDesc_t *desc = (PCMFileDesc_t*)handle;

    int ret = PCMReadOneFrameFromBuf(desc);
    while( ret < 0 )
    {
        ret = PCMReadData(desc);
        if( ret < 0 ) 
        {
            printf("ReadData error \n");
            return -1;
        }
        
        ret = PCMReadOneFrameFromBuf(desc);
        if( ret < 0 ) 
        {
            printf("ReadOneNaluFromBuf error \n");
//            return -1;
        }
        else
        {
            break;
        }
    }
    *length = desc->frame.size();
    *frame = desc->frame.data();

    return 0;
}

///////////////////////////////////////////////////////////
void* PCMDemux_Init(const char* filepath, uint32_t bitcount, uint32_t channelcount,
    uint32_t samplerate, PCMType pcmtype, int circleread)
{
    int i = 0, _samplerate = -1;
    for(;i< 16; i++)
    {
        if( pcm_sample_rates[i] == samplerate )
             break;
    }
    if( i == 16 )
    {
        printf("samplerate not support \n");
        return NULL;
    }

    PCMFileDesc_t* desc = new PCMFileDesc_t;
    desc->config.samplerateindex = i;
    desc->fs.open(filepath);
    if( !desc->fs.is_open() )
    {
        printf("file is not opened \n");
        return NULL;
    }

    desc->pcmType = pcmtype;
    desc->circleread = circleread;
    desc->config.channelcount = channelcount;
    desc->config.bitcount = bitcount;
    desc->config.samplerate = samplerate;
    desc->config.profilelevel = 2;

    uint32_t framerate = 25;
    uint32_t pcmFrameLength = samplerate * (bitcount / 8) * channelcount / framerate;
    if( pcmtype == PCM_G711A || pcmtype == PCM_G711U )
        pcmFrameLength /= 2;
    desc->defaultframelength = pcmFrameLength;//4096;

    return desc;
}

int PCMDemux_GetConfig(void *handle, PCMConfiguration_t *config)
{
    PCMFileDesc_t *desc = (PCMFileDesc_t*)handle;

    *config = desc->config;
    return 0;
}

int PCMDemux_GetFrame(void* handle, const char **frame, int *length)  
{
    return _PCMGetFrame(handle, frame, length);
}

int PCMDemux_CLose(void *handle)
{
    PCMFileDesc_t *desc = (PCMFileDesc_t*)handle;
    desc->fs.close();
    delete desc;

    return 0;    
}
