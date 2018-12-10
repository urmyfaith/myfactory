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
#include "rtpmux_raw.h"  

typedef struct
{
    std::string rtp_buffer;
    int packet_length;
    int last_packet_length;
    int packet_count;

    uint16_t sequence_number;
    uint32_t timestamp_increse;
    uint32_t timestamp_current;
    uint32_t rtp_ssrc;
}rtp_raw_mux_desc_t;

////////////////////////////////////////////////////////
void* rtpmux_raw_alloc(unsigned long ssrc)
{
    rtp_raw_mux_desc_t* handle = new rtp_raw_mux_desc_t;
    handle->packet_length = sizeof(RTP_FIXED_HEADER) + MAX_RTP_BODY_LENGTH;//1460;
    handle->last_packet_length = 0;
    handle->sequence_number = 0;
    handle->timestamp_increse = 90000/25;
    handle->timestamp_current = 0;
    handle->rtp_ssrc = ssrc;

    return (void*)handle;
}

int rtpmux_raw_setframe(void* handle, int streamtype, const char* frame_buffer, int frame_length)
{
    rtp_raw_mux_desc_t* rtp_mux = (rtp_raw_mux_desc_t*)handle;
    rtp_mux->rtp_buffer.clear();

   //设置RTP HEADER，  
    RTP_FIXED_HEADER rtp_hdr;      
    memset(&rtp_hdr,0,sizeof(rtp_hdr));  
    rtp_hdr.payload     = streamtype;  //负载类型号，  
    rtp_hdr.version     = 2;  //版本号，此版本固定为2  
    rtp_hdr.ssrc     = htonl(rtp_mux->rtp_ssrc);    //随机指定为10，并且在本RTP会话中全局唯一  
    
    //  当一个NALU小于1400字节的时候，采用一个单RTP包发送  
    if(frame_length <= MAX_RTP_BODY_LENGTH)  
    {
        //设置rtp M 位；  
        rtp_hdr.marker = 1;  
        rtp_hdr.seq_no  = htons(rtp_mux->sequence_number++); //序列号，每发送一个RTP包增1，htons，将主机字节序转成网络字节序。  

        rtp_hdr.timestamp=htonl(rtp_mux->timestamp_current);  
        rtp_mux->timestamp_current += rtp_mux->timestamp_increse;  

        rtp_mux->rtp_buffer.append((char*)&rtp_hdr, sizeof(rtp_hdr));

//        printf("sizeof(rtp_hdr)=%d %d \n", sizeof(rtp_hdr), rtp_mux->rtp_buffer.size());
        //NAL单元的第一字节和RTP荷载头第一个字节重合
        rtp_mux->rtp_buffer.append(frame_buffer, frame_length);

        rtp_mux->packet_count = 1;
        rtp_mux->last_packet_length = sizeof(rtp_hdr) + frame_length;
    }
    else
    {
        rtp_mux->packet_count = 0;

        rtp_hdr.timestamp=htonl(rtp_mux->timestamp_current);  
        rtp_mux->timestamp_current += rtp_mux->timestamp_increse;  
        while( frame_length > 0 )
        {
            rtp_hdr.seq_no = htons(rtp_mux->sequence_number++); //序列号，每发送一个RTP包增1  
            if(frame_length <= MAX_RTP_BODY_LENGTH)//发送的是最后一个分片，注意最后一个分片的长度可能超过1400字节（当 l> 1386时）。  
            {
                //设置rtp M 位；当前传输的是最后一个分片时该位置1  
                rtp_hdr.marker = 1;  
                rtp_mux->rtp_buffer.append((char*)&rtp_hdr, sizeof(rtp_hdr));

                rtp_mux->rtp_buffer.append(frame_buffer, frame_length);
                rtp_mux->last_packet_length = sizeof(rtp_hdr) + frame_length;
            }  
            //既不是第一个分片，也不是最后一个分片的处理。  
            else if(frame_length > MAX_RTP_BODY_LENGTH)  
            {  
                //设置rtp M 位；  
                rtp_hdr.marker = 0;  
                rtp_mux->rtp_buffer.append((char*)&rtp_hdr, sizeof(rtp_hdr));

                rtp_mux->rtp_buffer.append(frame_buffer, MAX_RTP_BODY_LENGTH);
            }

            rtp_mux->packet_count++;
            frame_length -= MAX_RTP_BODY_LENGTH;
            if( frame_length > 0 )
                frame_buffer+= MAX_RTP_BODY_LENGTH;
        }
    }

    return 0;    
}

int rtpmux_raw_getpacket(void* handle, const char **rtp_buffer, int *rtp_packet_length,
            int *rtp_last_packet_length, int *rtp_packet_count)
{
    rtp_raw_mux_desc_t* rtp_mux = (rtp_raw_mux_desc_t*)handle;
    *rtp_buffer = rtp_mux->rtp_buffer.data();
    *rtp_packet_length = rtp_mux->packet_length;
    *rtp_last_packet_length = rtp_mux->last_packet_length;
    *rtp_packet_count = rtp_mux->packet_count;

    return 0;
}

int rtpmux_raw_free(void* handle)
{
    rtp_raw_mux_desc_t* rtp_mux = (rtp_raw_mux_desc_t*)handle;
    delete rtp_mux;
    return 0;
}