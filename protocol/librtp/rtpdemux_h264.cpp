#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <sys/types.h>  
#include <unistd.h>  
#include <errno.h>

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <sys/socket.h>  
    #include <netinet/in.h>  
    #include <arpa/inet.h>  
#endif

#include <string>
#include "rtpdemux_h264.h"  

#define LOOPTIMESTAMPCORNER 10000

typedef struct
{
    std::string rtp_buffer;

    RTP_FIXED_HEADER rtpheader;
    rtpdemux_h264frame_callback framecallback;
}rtp_h264_demux_desc_t;

#pragma pack(push,1)
typedef struct {  
    //byte 0  
    unsigned char TYPE:5;  
    unsigned char NRI:2;  
    unsigned char F:1;
} NALU_HEADER; /**//* 1 BYTES */  
  
typedef struct {  
    //byte 0  
    unsigned char TYPE:5;  
    unsigned char NRI:2;   
    unsigned char F:1;      
} FU_INDICATOR; /**//* 1 BYTES */  
  
typedef struct {  
    //byte 0  
    unsigned char TYPE:5;  
    unsigned char R:1;  
    unsigned char E:1;  
    unsigned char S:1;      
} FU_HEADER; /**//* 1 BYTES */        
#pragma pack(pop)

int rtpheader_parse(char* buffer, RTP_FIXED_HEADER *header)
{
    if( !buffer || !header )
        return -1;

    memcpy(header, buffer, sizeof(RTP_FIXED_HEADER));
    header->seq_no = ntohs(header->seq_no);
    header->timestamp = ntohl(header->timestamp);
    header->ssrc = ntohl(header->ssrc);

//    printf("rtpheader_parse: seq_no=%u, timestamp=%u, ssrc=%u \n", 
//            header->seq_no, header->timestamp, header->ssrc);

    return 0;
}

////////////////////////////////////////////////////////
void* rtpdemux_h264_alloc(rtpdemux_h264frame_callback callback)
{
    rtp_h264_demux_desc_t* handle = new rtp_h264_demux_desc_t;

    handle->rtp_buffer.clear();
    handle->framecallback =callback;
    return (void*)handle;
}

int rtpdemux_h264_setpacket(void* handle, char* packet_buffer, int packet_length)
{
    if( packet_length < sizeof(RTP_FIXED_HEADER) + 2 )
    {
        printf("rtpdemux_h264_setpacket: packet length error\n");
        return -1;
    }

    rtp_h264_demux_desc_t* rtp_demux = (rtp_h264_demux_desc_t*)handle;

    RTP_FIXED_HEADER header;
    rtpheader_parse(packet_buffer, &header);
    if( rtp_demux->rtp_buffer.size() <= 0 )
        rtp_demux->rtpheader = header;

    if( rtp_demux->rtpheader.timestamp < header.timestamp ||
        (rtp_demux->rtpheader.timestamp > LOOPTIMESTAMPCORNER && header.timestamp < LOOPTIMESTAMPCORNER))
    {
        rtp_demux->framecallback(rtp_demux->rtp_buffer.data(), rtp_demux->rtp_buffer.size(), 
                rtp_demux->rtpheader.timestamp, rtp_demux->rtpheader.ssrc);
        rtp_demux->rtp_buffer.clear();            
        rtp_demux->rtpheader = header;
    }

    if(rtp_demux->rtpheader.timestamp != header.timestamp)
    {
        printf("litter late timestamp rtp packet \n");
        return -1;
    }

    char* buffer = packet_buffer+sizeof(RTP_FIXED_HEADER);
    uint32_t length = packet_length - sizeof(RTP_FIXED_HEADER);

    char h264prefix[] = {0, 0, 0, 1};
    NALU_HEADER checkheader = *(NALU_HEADER*)buffer;
    if( checkheader.TYPE == 7 || checkheader.TYPE == 8 || checkheader.TYPE == 6 ||
        checkheader.TYPE == 5 || checkheader.TYPE == 1 )
    {
        rtp_demux->rtp_buffer.append(h264prefix, sizeof(h264prefix));
        rtp_demux->rtp_buffer.append(buffer, length);
    }
    else if( checkheader.TYPE == 28)//FU-A类型
    {
        buffer++;
        length--;
        FU_HEADER fuheader = *(FU_HEADER*)buffer;
        checkheader.TYPE = fuheader.TYPE;
        if( rtp_demux->rtp_buffer.size() <= 0 )
        {
            rtp_demux->rtp_buffer.append(h264prefix, sizeof(h264prefix));
            rtp_demux->rtp_buffer.append((char*)&checkheader, sizeof(checkheader));
        }

        buffer++;
        length--;
        rtp_demux->rtp_buffer.append(buffer, length);
    }

    return 0;    
}

int rtpdemux_h264_free(void* handle)
{
    rtp_h264_demux_desc_t* rtp_demux = (rtp_h264_demux_desc_t*)handle;
    delete rtp_demux;
    return 0;
}