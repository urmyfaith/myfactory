#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <errno.h>
#include <fstream>
#include <string>
#include <vector>

#include "aacdemux.h"  

static const char *aac_profiles[] = {"Main", "LC", "SSR", "LDT"};

static int aac_sample_rates[16] =
{
  96000, 88200, 64000, 48000, 44100, 32000,
  24000, 22050, 16000, 12000, 11025, 8000, 7350,
  0, 0, 0
};

typedef struct {
    AACConfiguration_t config;
    
    std::fstream fs;

    std::string databuffer;
    char buffer[1024 * 1024];

    std::string frame;
    std::vector<std::string> probeframes;

    int circleread;
}AACFileDesc_t;

int AACParseFrame(char *buffer, int buflength, int &framelength, int &startpos)
{
    if(buflength  < 9 ){ ///< ATDS header长度为7 or 9
        return -1;
    }

    int i = 0, length = 0;
    for( i = 0; i < buflength-9; i++)
    {
        //Sync words
        if(((uint8_t)buffer[i] == (uint8_t)0xff) && 
                ((uint8_t)(buffer[i+1] & 0xf0) == (uint8_t)0xf0) ){
            length |= ((buffer[i+3] & 0x03) <<11);     //high 2 bit
            length |= buffer[i+4]<<3;                //middle 8 bit
            length |= ((buffer[i+5] & 0xe0)>>5);        //low 3bit

//            printf("got header %d %d\n", length, buflength);
            break;
        }
    }

    if(i == buflength-9 || buflength < length || length < 9 ){
        printf("AACParseFrame internal error %d %d\n", length, buflength);
        return -1;
    }

    startpos = i;
    framelength = length;

    return 0;
}

int AACReadOneFrameFromBuf(void* handle)
{
    AACFileDesc_t *desc = (AACFileDesc_t*)handle;

    int framelength = -1, startpos = -1;
    int ret = AACParseFrame((char*)desc->databuffer.data(), desc->databuffer.size(), framelength, startpos);
    if( ret < 0 ) 
    {
        printf("AACParseFrame error \n");
        return -1;
    }

//    printf("framelength = %d\n", framelength);

    desc->frame.clear();
    desc->frame.append(desc->databuffer.data()+startpos, framelength);

    desc->databuffer.erase(0, startpos+framelength);

    return framelength;
}

int AACReadData(void* handle)
{
    AACFileDesc_t *desc = (AACFileDesc_t*)handle;

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

///////////////////////////////////////////////////////////
void* AACDemux_Init(const char* filepath, int circleread) 
{
    AACFileDesc_t* desc = new AACFileDesc_t;
    desc->fs.open(filepath);
    if( !desc->fs.is_open() )
    {
        printf("file is not opened \n");
        return NULL;
    }

    desc->circleread = circleread;
    desc->config.headerlen = 7;

    return desc;
}

int AACDemux_GetConfig(void *handle, AACConfiguration_t *config)
{
    AACFileDesc_t *desc = (AACFileDesc_t*)handle;

    const char *frame = NULL;
    int framelength = -1;
    if( AACDemux_GetFrame(handle, &frame, &framelength) < 0 )
    {
        printf("AACDemux_GetConfig GetFrame error\n");
        return -1;
    }
    desc->probeframes.push_back(std::string(frame, framelength));

    *config = desc->config;
    
    return 0;
}

int AACDemux_GetFrame(void* handle, const char **frame, int *length)  
{
    AACFileDesc_t *desc = (AACFileDesc_t*)handle;
    
    if( !desc->probeframes.empty() )
    {
        desc->frame = desc->probeframes[0];
        desc->probeframes.erase(desc->probeframes.begin());

        *frame = desc->frame.data();
        *length = desc->frame.size();
        return 0;
    }

    int ret = AACReadOneFrameFromBuf(desc);
    while( ret < 0 )
    {
        ret = AACReadData(desc);
        if( ret < 0 ) 
        {
            printf("ReadData error \n");
            return -1;
        }
        
        ret = AACReadOneFrameFromBuf(desc);
        if( ret < 0 ) 
        {
            printf("AACReadOneFrameFromBuf error \n");
        }
        else
        {
            break;
        }
    }

    uint8_t profile = desc->frame[2]&0xC0;
    desc->config.profilelevel = (profile>>6) + 1;
    
    uint8_t sampling_frequency_index = desc->frame[2]&0x3C;
    desc->config.samplerate = aac_sample_rates[sampling_frequency_index>>2];
    desc->config.samplerateindex = (unsigned char)sampling_frequency_index>>2;
    desc->config.channelcount = (((unsigned char)desc->frame[2] & 0x01) << 2) + ((unsigned char)desc->frame[3]>>6);

//    desc->frame.erase(0,7);
    *frame = desc->frame.data();
    *length = desc->frame.size();

//    printf("profilelevel %d samplerate %d \n", desc->profilelevel, desc->samplerate);

    return 0;
}

int AACDemux_CLose(void *handle)
{
    AACFileDesc_t *desc = (AACFileDesc_t*)handle;
    desc->fs.close();
    delete desc;

    return 0;    
}
