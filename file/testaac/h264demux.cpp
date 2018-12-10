#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <errno.h>
#include <fstream>
#include <string>
#include <vector>

#include "h264demux.h"  
#include "spspps.h"

typedef struct {
    H264Configuration_t config;

    std::fstream fs;

    std::string databuffer;
    char buffer[1024 * 1024];

    std::string sps;
    std::string pps;

    std::string frame;
    std::vector<std::string> probeframes;

    int circleread;
}H264FileDesc_t;

int H264ParseFrame(char *buffer, int buflength, int &framelength, int &startpos) 
{
    int nFirstPos = -1;
    int i = 0;
    for( i = 0; i < buflength-4; i++)
    {
        if ( buffer[i] == 0 && buffer[i+1] == 0 && buffer[i+2] == 1 ) 
        {
            nFirstPos = i+3;
            i+=3;
            break;
        }
    }
    if( nFirstPos < 0 )
    {
//      println("nFirstPos < 0 ")
        return -1;
    }

    int nEndPos = -1;
    for( ; i < buflength-4; i++)
    {
        if ( buffer[i] == 0 && buffer[i+1] == 0 &&
             (buffer[i+2] == 1 || (buffer[i+2] == 0 && buffer[i+3] == 1)) ) 
        {
            nEndPos = i;
            break;
        }
    }
    if( nEndPos < 0)
    {
//      println("nEndPos < 0 ")
        return -1;
    }

    startpos = nFirstPos;
    framelength = nEndPos - nFirstPos;

    return 0;
}

int H264ReadOneFrameFromBuf(void* handle)
{
    H264FileDesc_t *desc = (H264FileDesc_t*)handle;

    int framelength = -1, startpos = -1;
    int ret = H264ParseFrame((char*)desc->databuffer.data(), desc->databuffer.size(), framelength, startpos);
    if( ret < 0 ) 
    {
        printf("H264ParseFrame error \n");
        return -1;
    }

    desc->frame.clear();
    char prefix[] = {0,0,0,1};
    desc->frame.append(prefix, 4);
    desc->frame.append(desc->databuffer.data()+startpos, framelength);

    desc->databuffer.erase(0, startpos+framelength);
//    memmove(desc->databuffer, desc->databuffer+startpos+framelength, desc->offset - startpos - framelength);

    return framelength;
}

int H264ReadData(void* handle)
{
    H264FileDesc_t *desc = (H264FileDesc_t*)handle;

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

int _GetFrame(void* handle, const char **frame, int *length)  
{
    H264FileDesc_t *desc = (H264FileDesc_t*)handle;

    int ret = H264ReadOneFrameFromBuf(desc);
    while( ret < 0 )
    {
        ret = H264ReadData(desc);
        if( ret < 0 ) 
        {
            printf("ReadData error \n");
            return -1;
        }
        
        ret = H264ReadOneFrameFromBuf(desc);
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

    int frametype = desc->frame[4] & 0x1F;
//    printf("frametype %d \n", frametype);
    if( frametype == 7 )
    {
        desc->sps.clear();
        desc->sps.append((char*)desc->frame.data(), desc->frame.size());
        desc->config.sps = desc->sps.data();
        desc->config.spslen = desc->sps.size();

        SPS stuSps;
        h264dec_seq_parameter_set((void*)desc->sps.data()+5, desc->sps.size()-5, &stuSps);
        desc->config.width = (stuSps.pic_width_in_mbs_minus1+1)*16;
        desc->config.height = (stuSps.pic_height_in_map_units_minus1+1)*16;

        if( stuSps.vui_parameters.timing_info_present_flag )
        {
            int fps = stuSps.vui_parameters.time_scale / stuSps.vui_parameters.num_units_in_tick;
//            if( stuSps.vui_parameters.fixed_frame_rate_flag )
            if( fps > 10 && fps <50 )
            {
                desc->config.framerate = fps/2;
            }                    
        }
//        printf("sps width=%d height=%d fps=%d timescale %d========\n ", 
//            desc->config.width, desc->config.height, desc->config.framerate, 
//            stuSps.vui_parameters.time_scale);
    }
    else if( frametype == 8 )
    {
        desc->pps.clear();
        desc->pps.append((char*)desc->frame.data(), desc->frame.size());

        desc->config.pps = desc->pps.data();
        desc->config.ppslen = desc->pps.size();

//        printf("got pps \n");
    }

    return 0;
}

///////////////////////////////////////////////////////////
void* H264Demux_Init(const char* filepath, int circleread) 
{
    H264FileDesc_t* desc = new H264FileDesc_t;
    desc->fs.open(filepath);
    if( !desc->fs.is_open() )
    {
        printf("file is not opened \n");
        return NULL;
    }

    desc->config.framerate = 25;
    desc->config.timescale = 90000;
    desc->circleread = circleread;
    desc->config.headerlen = 4;
    return desc;
}

int H264Demux_GetConfig(void *handle, H264Configuration_t *config)
{
    H264FileDesc_t *desc = (H264FileDesc_t*)handle;
    while( desc->sps.size() <= 0 || desc->pps.size() <= 0)
    {
        const char *frame = NULL;
        int framelength = -1;
        if( _GetFrame(handle, &frame, &framelength) < 0 )
        {
            printf("H264Demux_GetConfig GetFrame error\n");
            return -1;
        }

        printf("H264Demux_GetConfig %d %d \n", desc->sps.size(), desc->pps.size());
        desc->probeframes.push_back(std::string(frame, framelength));        
    }

    *config = desc->config;
    return 0;
}

int H264Demux_GetFrame(void* handle, const char **frame, int *length)  
{
    H264FileDesc_t *desc = (H264FileDesc_t*)handle;

    if( !desc->probeframes.empty() )
    {
        desc->frame = desc->probeframes[0];
        desc->probeframes.erase(desc->probeframes.begin());

        *frame = desc->frame.data();
        *length = desc->frame.size();
        return 0;
    }

    return _GetFrame(handle, frame, length);
}

int H264Demux_CLose(void *handle)
{
    H264FileDesc_t *desc = (H264FileDesc_t*)handle;
    desc->fs.close();
    delete desc;

    return 0;    
}
