#ifndef __RTP_H__
#define __RTP_H__
             
#include <stdint.h>

#define MAX_RTP_BODY_LENGTH     1440    
#define H264                    96  
#define AAC                    97  
#define G711_PCMU                0  
#define G711_PCMA                8  
#define G729_PCM                18  

#pragma pack(push,1)
typedef struct   
{  
    /**//* byte 0 */  
    unsigned char csrc_len:4;        /**//* expect 0 */  
    unsigned char extension:1;        /**//* expect 1, see RTP_OP below */  
    unsigned char padding:1;        /**//* expect 0 */  
    unsigned char version:2;        /**//* expect 2 */  
    /**//* byte 1 */  
    unsigned char payload:7;        /**//* RTP_PAYLOAD_RTSP */  
    unsigned char marker:1;        /**//* expect 1 */  
    /**//* bytes 2, 3 */  
    uint16_t seq_no;              
    /**//* bytes 4-7 */  
    uint32_t timestamp;          
    /**//* bytes 8-11 */  
    uint32_t ssrc;            /**//* stream number is used here. */  
} RTP_FIXED_HEADER;  
#pragma pack(pop)

int get_local_rtp_rtcp_port(int *rtp_sock, int *rtp_port, int *rtcp_sock, int *rtcp_port);

#endif