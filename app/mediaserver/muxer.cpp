#include <stdio.h>

#include <string>

#include "muxer.h"
#include "../../protocol/librtp/rtph264.h"
#include "../../protocol/librtp/rtpaac.h"
#include "../../protocol/librtp/rtpraw.h"

struct muxer_tag_t
{
    void* muxer_handle;
    
    std::string sourcetype;

    std::string framedata;
};

///////////////////////////////////////////////////////////////////
void* muxer_alloc(const char* sourcetype)
{
    std::string temp = sourcetype;
     void* muxer_handle = NULL;
    if( temp == "264" )
    {
        muxer_handle = rtpmux_h264_alloc(1);
    }
    else if( temp == "aac" )
    {
        muxer_handle = rtpmux_aac_alloc(1);
    }
    else if( temp == "alaw" || temp == "mulaw" )
    {
        muxer_handle = rtpmux_raw_alloc(1);        
    }

    if( !muxer_handle )
    {
        printf("alloc framer_handle error, sourcetype:%s \n", sourcetype);
        return NULL;
    }

    muxer_tag_t *inst = new muxer_tag_t;
    inst->sourcetype = sourcetype;
    inst->muxer_handle = muxer_handle;

    return inst;
}

int muxer_setframe(void* handle, const char* h264frame, int framelength)
{
    muxer_tag_t *inst = (muxer_tag_t*)handle;
    if( inst->sourcetype == "264" )
    {
        return rtpmux_h264_setframe(inst->muxer_handle, h264frame, framelength);
    }
    else if( inst->sourcetype == "aac" )
    {
        return rtpmux_aac_setframe(inst->muxer_handle, h264frame, framelength);
    }
    else if( inst->sourcetype == "alaw" )
    {
        return rtpmux_raw_setframe(inst->muxer_handle, G711_PCMA, h264frame, framelength);
    }
    else if( inst->sourcetype == "mulaw" )
    {
        return rtpmux_raw_setframe(inst->muxer_handle, G711_PCMU, h264frame, framelength);
    }

    return -1;
}


int muxer_getpacket(void* handle, const char** rtp_buffer, 
    int *rtp_packet_length, int *last_rtp_packet_length, int *rtp_packet_count)
{
    muxer_tag_t *inst = (muxer_tag_t*)handle;
    if( inst->sourcetype == "264" )
    {
        return rtpmux_h264_getpacket(inst->muxer_handle, rtp_buffer, rtp_packet_length, last_rtp_packet_length, rtp_packet_count);
    }
    else if( inst->sourcetype == "aac" )
    {
        return rtpmux_aac_getpacket(inst->muxer_handle, rtp_buffer, rtp_packet_length, last_rtp_packet_length, rtp_packet_count);
    }
    else if( inst->sourcetype == "alaw" || inst->sourcetype == "mulaw" )
    {
        return rtpmux_raw_getpacket(inst->muxer_handle, rtp_buffer, rtp_packet_length, last_rtp_packet_length, rtp_packet_count);
    }

    return -1;
}

int muxer_free(void *handle)
{
    muxer_tag_t *inst = (muxer_tag_t*)handle;
    if( inst->sourcetype == "264" && inst->muxer_handle )
    {
        rtpmux_h264_free(inst->muxer_handle);
    }
    else if( inst->sourcetype == "aac" && inst->muxer_handle )
    {
        rtpmux_aac_free(inst->muxer_handle);
    }
    else if( inst->sourcetype == "alaw" || inst->sourcetype == "mulaw" )
    {
        rtpmux_raw_free(inst->muxer_handle);
    }

    delete inst;

    return 0;
}
