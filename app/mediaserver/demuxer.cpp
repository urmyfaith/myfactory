#include <stdio.h>

#include <string>

#include "demuxer.h"
#include "../../file/libh26x/h264demux.h"
#include "../../file/libaac/aacdemux.h"
#include "../../file/libpcm/pcmdemux.h"

struct demuxer_tag_t
{
    void* framer_handle;
    
    std::string sourcetype;
    std::string sourcename;

    std::string framedata;
};

void* h264framer_alloc(const char* sourcename, H264Configuration_t *config)
{
    void* h264handle = H264Demux_Init((char*)sourcename, 1);
    if( !h264handle )
    {
        printf("H264Framer_Init error\n");
        return NULL;
    }
    
    if( H264Demux_GetConfig(h264handle, config) < 0 )
    {
        printf("H264Demux_GetConfig error\n");
        H264Demux_CLose(h264handle);
        return NULL;
    }
    printf("H264Demux_GetConfig:width %d height %d framerate %d timescale %d %d %d \n",
        config->width, config->height, config->framerate, config->timescale,
        config->spslen, config->ppslen);

    return h264handle;
}

int h264framer_getframe(void* handle, const char** frame, int *length)
{
    demuxer_tag_t *inst = (demuxer_tag_t*)handle;
    const char *h264frame = NULL;
    int framelength = -1;
    int ret = H264Demux_GetFrame(inst->framer_handle, &h264frame, &framelength);
    if( ret < 0 )
    {
        printf("ReadOneNaluFromBuf error\n");
        return -1;
    }

    int frametype = h264frame[4]&0x1f;
/*
    printf("frameinfo:%u %u %u %u %u frametype:%d framelength:%d \n", 
          h264frame[0], h264frame[1], h264frame[2], h264frame[3],h264frame[4], 
          frametype, framelength);
*/
    if( frametype == 5 )
    {
        H264Configuration_t config;
        H264Demux_GetConfig(inst->framer_handle, &config);
        inst->framedata.clear();
        inst->framedata.append(config.sps+4, config.spslen-4);
        inst->framedata.append(config.pps, config.ppslen);
        inst->framedata.append(h264frame, framelength);
    }
    else
    {
        inst->framedata.clear();
        inst->framedata.append(h264frame+4, framelength-4);
    }
    *frame = inst->framedata.data();
    *length = inst->framedata.size();

    return 0;
}

///////////////////////////////////////////////////////////////////
void* demuxer_alloc(const char* sourcename)
{
    std::string temp = sourcename;
    int pos = temp.rfind(".");
    if( pos < 0 )
        return NULL;

    std::string sourcetype = temp.substr(pos+1);
    void* framer_handle = NULL;
    if( sourcetype == "264" )
    {
        H264Configuration_t h264config;
        framer_handle = h264framer_alloc(sourcename, &h264config);
    }
    else if( sourcetype == "aac" )
    {
        framer_handle = AACDemux_Init(sourcename, 1);
    }
    else if( sourcetype == "alaw" )
    {
        framer_handle = PCMDemux_Init(sourcename, 16, 2, 44100, PCM_G711A, 1);
    }
    else if( sourcetype == "mulaw" )
    {
        framer_handle = PCMDemux_Init(sourcename, 16, 2, 44100, PCM_G711U, 1);
    }

    if( !framer_handle )
    {
        printf("alloc framer_handle error, sourcename:%s, sourcetype:%s \n", sourcename, sourcetype.c_str());
        return NULL;
    }

    demuxer_tag_t *inst = new demuxer_tag_t;
    inst->sourcename = sourcename;
    inst->sourcetype = sourcetype.c_str();
    inst->framer_handle = framer_handle;

    return inst;
}

int demuxer_getframe(void* handle, const char** frame, int *length)
{
    demuxer_tag_t *inst = (demuxer_tag_t*)handle;
    if( inst->sourcetype == "264" )
    {
        return h264framer_getframe(handle, frame, length);
    }
    else if( inst->sourcetype == "aac" )
    {
        return AACDemux_GetFrame(inst->framer_handle, frame, length);
    }
    else if( inst->sourcetype == "alaw" || inst->sourcetype == "mulaw" )
    {
        return PCMDemux_GetFrame(inst->framer_handle, frame, length);        
    }

    return 0;
}

int demuxer_free(void *handle)
{
    demuxer_tag_t *inst = (demuxer_tag_t*)handle;
    if( inst->sourcetype == "264" )
    {
        H264Demux_CLose(inst->framer_handle);
    }
    else if( inst->sourcetype == "aac" )
    {
        AACDemux_CLose(inst->framer_handle);
    }
    else if( inst->sourcetype == "alaw" || inst->sourcetype == "mulaw" )
    {
        PCMDemux_CLose(inst->framer_handle);        
    }

    if( inst )
        delete inst;

    return 0;
}