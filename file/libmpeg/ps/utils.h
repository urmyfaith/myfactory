#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#define PS_HDR_LEN  14  
#define SYS_HDR_LEN 18  
#define PSM_HDR_LEN 20//24  
#define PES_HDR_LEN 19  
#define RTP_HDR_LEN 12  
#define RTP_VERSION 1

#define RTP_MAX_PACKET_BUFF 1300
#define PS_PES_PAYLOAD_SIZE 65535-50

struct ps_stream_info{
    int streamTypeID;
    int streamType;
};

/* @remark: 结构体定义 */  
typedef struct  
{  
    int i_size;             // p_data字节数  
    int i_data;             // 当前操作字节的位置  
    unsigned char i_mask;   // 当前操作位的掩码  
    unsigned char *p_data;  // bits buffer  
} BITS_BUFFER_S;

/* remark:接口函数定义 */  
int bits_initwrite(BITS_BUFFER_S *p_buffer, int i_size, unsigned char *p_data);
  
void bits_align(BITS_BUFFER_S *p_buffer);
  
void bits_write(BITS_BUFFER_S *p_buffer, int i_count, unsigned long i_bits);
    
int bits_initread(BITS_BUFFER_S *p_buffer, int i_size, unsigned char *p_data);
  
int bits_read(BITS_BUFFER_S *p_buffer, int i_count, unsigned long *i_bits);

// CRC32
uint32_t CRC_encode(const char* data, int len);