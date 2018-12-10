#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>
#include <sys/select.h>

#include "utils.h" 

int is_little_endian(void)
{
  unsigned short flag=0x4321;
  if (*(unsigned char*)&flag==0x21)
    return 1;
  else
    return 0;
}

uint64_t __ntohll2(uint64_t val)
{
    if( is_little_endian() )
    {
        return (((uint64_t)htonl((int32_t)((val << 32) >> 32))) << 32) | (uint32_t)htonl((int32_t)(val >> 32));
    }
    else
    {
        return val;
    }
}

uint64_t __htonll2(uint64_t val)
{
    if( is_little_endian() )
    {
        return (((uint64_t)htonl((int32_t)((val << 32) >> 32))) << 32) | (uint32_t)htonl((int32_t)(val >> 32));
    }
    else
    {
        return val;
    }
}

const char* getpayloadtypestr(DMIPAYLOADTYPE payloadtype)
{
    switch( payloadtype )
    {
        case CODEC_H264:
            return "CODEC_H264";
        case CODEC_H265:
            return "CODEC_H265";
        case CODEC_VP8:
            return "CODEC_VP8";
        case CODEC_VP9:
            return "CODEC_VP9";
        case CODEC_MJPEG:
            return "CODEC_MJPEG";
        case CODEC_PNG:
            return "CODEC_PNG";
        case CODEC_AAC:
            return "CODEC_AAC";
        case CODEC_G711:
            return "CODEC_G711";
        case CODEC_JSON:
            return "CODEC_JSON";
        case CODEC_PROTOBUF:
            return "CODEC_PROTOBUF";
        case CODEC_TEXT:
            return "CODEC_TEXT";
        case CODEC_DMIC_REQUEST:
            return "CODEC_DMIC_REQUEST";
        case CODEC_DMIC_RESPONSE:
            return "CODEC_DMIC_RESPONSE";
        case CODEC_UNKNOWN:
            return "CODEC_UNKNOWN";
        default:
            return "";
    }
}