#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <unistd.h>

#define PS_HDR_LEN  14  
#define SYS_HDR_LEN 18  
#define PSM_HDR_LEN 20//24  
#define PES_HDR_LEN 19  
#define RTP_HDR_LEN 12  
#define RTP_VERSION 1

#define RTP_MAX_PACKET_BUFF 1300
#define PS_PES_PAYLOAD_SIZE 65535-50

std::fstream fsps;
std::fstream fs264;
FILE *bits = NULL;                //!< the bit stream file  

typedef struct  
{  
    int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)  
    unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)  
    unsigned max_size;            //! Nal Unit Buffer size  
    int forbidden_bit;            //! should be always FALSE  
    int nal_reference_idc;        //! NALU_PRIORITY_xxxx  
    int nal_unit_type;            //! NALU_TYPE_xxxx      
    char *buf;                    //! contains the first byte followed by the EBSP  
    unsigned short lost_packets;  //! true, if packet loss is detected  
} NALU_t;  

/* @remark: 结构体定义 */  
typedef struct  
{  
    int i_size;             // p_data字节数  
    int i_data;             // 当前操作字节的位置  
    unsigned char i_mask;   // 当前操作位的掩码  
    unsigned char *p_data;  // bits buffer  
} bits_buffer_s;  

struct Data_Info_s 
{
    unsigned long long s64CurPts;
    int IFrame;
    unsigned short u16CSeq;
    unsigned int u32Ssrc;
    char szBuff[RTP_MAX_PACKET_BUFF];
};

/*** 
 *@remark:  讲传入的数据按地位一个一个的压入数据 
 *@param :  buffer   [in]  压入数据的buffer 
 *          count    [in]  需要压入数据占的位数 
 *          bits     [in]  压入的数值 
 */  
inline void bits_write(bits_buffer_s *p_buffer, int i_count, unsigned long i_bits)  
{  
    while (i_count > 0)  
    {  
        i_count--;  
  
        if ((i_bits >> i_count ) & 0x01)  
        {  
            p_buffer->p_data[p_buffer->i_data] |= p_buffer->i_mask;  
        }  
        else  
        {  
            p_buffer->p_data[p_buffer->i_data] &= ~p_buffer->i_mask;  
        }  
        p_buffer->i_mask >>= 1;  
        if (p_buffer->i_mask == 0)  
        {  
            p_buffer->i_data++;  
            p_buffer->i_mask = 0x80;  
        }  
    }  
}

/*** 
 *@remark:   ps头的封装,里面的具体数据的填写已经占位，可以参考标准 
 *@param :   pData  [in] 填充ps头数据的地址 
 *           s64Src [in] 时间戳 
 *@return:   0 success, others failed 
*/  
int gb28181_make_ps_header(char *pData, unsigned long long s64Scr)  
{  
    unsigned long long lScrExt = (s64Scr) % 100;      
    s64Scr = s64Scr / 100;  
    // 这里除以100是由于sdp协议返回的video的频率是90000，帧率是25帧/s，所以每次递增的量是3600,  
    // 所以实际你应该根据你自己编码里的时间戳来处理以保证时间戳的增量为3600即可，  
    //如果这里不对的话，就可能导致卡顿现象了  
    bits_buffer_s   bitsBuffer;  
    bitsBuffer.i_size = PS_HDR_LEN;   
    bitsBuffer.i_data = 0;  
    bitsBuffer.i_mask = 0x80; // 二进制：10000000 这里是为了后面对一个字节的每一位进行操作，避免大小端夸字节字序错乱  
    bitsBuffer.p_data = (unsigned char *)(pData);  
    memset(bitsBuffer.p_data, 0, PS_HDR_LEN);  
    bits_write(&bitsBuffer, 32, 0x000001BA);            /*start codes*/  
    bits_write(&bitsBuffer, 2,  1);                     /*marker bits '01b'*/  
    bits_write(&bitsBuffer, 3,  (s64Scr>>30)&0x07);     /*System clock [32..30]*/  
    bits_write(&bitsBuffer, 1,  1);                     /*marker bit*/  
    bits_write(&bitsBuffer, 15, (s64Scr>>15)&0x7FFF);   /*System clock [29..15]*/  
    bits_write(&bitsBuffer, 1,  1);                     /*marker bit*/  
    bits_write(&bitsBuffer, 15, s64Scr&0x7fff);         /*System clock [29..15]*/  
    bits_write(&bitsBuffer, 1,  1);                     /*marker bit*/  

    bits_write(&bitsBuffer, 9,  lScrExt&0x01ff);        /*System clock [14..0]*/  
    bits_write(&bitsBuffer, 1,  1);                     /*marker bit*/  

    bits_write(&bitsBuffer, 22, (255)&0x3fffff);        /*bit rate(n units of 50 bytes per second.)*/  
    bits_write(&bitsBuffer, 2,  3);                     /*marker bits '11'*/  

    bits_write(&bitsBuffer, 5,  0x1f);                  /*reserved(reserved for future use)*/  
    bits_write(&bitsBuffer, 3,  0);                     /*stuffing length*/  
    return 0;  
}

/*** 
 *@remark:   sys头的封装,里面的具体数据的填写已经占位，可以参考标准 
 *@param :   pData  [in] 填充ps头数据的地址 
 *@return:   0 success, others failed 
*/  
int gb28181_make_sys_header(char *pData)  
{  
      
    bits_buffer_s   bitsBuffer;  
    bitsBuffer.i_size = SYS_HDR_LEN;  
    bitsBuffer.i_data = 0;  
    bitsBuffer.i_mask = 0x80;  
    bitsBuffer.p_data = (unsigned char *)(pData);  
    memset(bitsBuffer.p_data, 0, SYS_HDR_LEN);  
    /*system header*/  
    bits_write( &bitsBuffer, 32, 0x000001BB);   /*start code*/  
    bits_write( &bitsBuffer, 16, SYS_HDR_LEN-6);/*header_length 表示次字节后面的长度，后面的相关头也是次意思*/  
    bits_write( &bitsBuffer, 1,  1);            /*marker_bit*/  
    bits_write( &bitsBuffer, 22, 50000);        /*rate_bound*/  
    bits_write( &bitsBuffer, 1,  1);            /*marker_bit*/  
    bits_write( &bitsBuffer, 6,  1);            /*audio_bound*/  
    bits_write( &bitsBuffer, 1,  0);            /*fixed_flag */  
    bits_write( &bitsBuffer, 1,  0);            /*CSPS_flag */  
//    bits_write( &bitsBuffer, 1,  1);            /*CSPS_flag */  
    bits_write( &bitsBuffer, 1,  1);            /*system_audio_lock_flag*/  
    bits_write( &bitsBuffer, 1,  1);            /*system_video_lock_flag*/  
    bits_write( &bitsBuffer, 1,  1);            /*marker_bit*/  
    bits_write( &bitsBuffer, 5,  1);            /*video_bound*/  
    bits_write( &bitsBuffer, 1,  0);            /*dif from mpeg1*/  
    bits_write( &bitsBuffer, 7,  0x7F);         /*reserver*/  
    /*audio stream bound*/  
    bits_write( &bitsBuffer, 8,  0xC0);         /*stream_id*/  
    bits_write( &bitsBuffer, 2,  3);            /*marker_bit */  
    bits_write( &bitsBuffer, 1,  0);            /*PSTD_buffer_bound_scale*/  
    bits_write( &bitsBuffer, 13, 512);          /*PSTD_buffer_size_bound*/  
    /*video stream bound*/  
    bits_write( &bitsBuffer, 8,  0xE0);         /*stream_id*/  
    bits_write( &bitsBuffer, 2,  3);            /*marker_bit */  
    bits_write( &bitsBuffer, 1,  1);            /*PSTD_buffer_bound_scale*/  
    bits_write( &bitsBuffer, 13, 2048);         /*PSTD_buffer_size_bound*/  
    return 0;  
}  

/*** 
 *@remark:   psm头的封装,里面的具体数据的填写已经占位，可以参考标准 
 *@param :   pData  [in] 填充ps头数据的地址 
 *@return:   0 success, others failed 
*/  
int gb28181_make_psm_header(char *pData)  
{
    bits_buffer_s   bitsBuffer;  
    bitsBuffer.i_size = PSM_HDR_LEN;   
    bitsBuffer.i_data = 0;  
    bitsBuffer.i_mask = 0x80;  
    bitsBuffer.p_data = (unsigned char *)(pData);  
    memset(bitsBuffer.p_data, 0, PSM_HDR_LEN);  
    bits_write(&bitsBuffer, 24,0x000001);   /*start code*/  
    bits_write(&bitsBuffer, 8, 0xBC);       /*map stream id*/  
    bits_write(&bitsBuffer, 16,PSM_HDR_LEN - 6);         /*program stream map length*/   
    bits_write(&bitsBuffer, 1, 1);          /*current next indicator */  
    bits_write(&bitsBuffer, 2, 3);          /*reserved*/  
    bits_write(&bitsBuffer, 5, 0);          /*program stream map version*/  
    bits_write(&bitsBuffer, 7, 0x7F);       /*reserved */  
    bits_write(&bitsBuffer, 1, 1);          /*marker bit */  
    bits_write(&bitsBuffer, 16,0);          /*programe stream info length*/  
    bits_write(&bitsBuffer, 16, 4);         /*elementary stream map length  is*/  
//    bits_write(&bitsBuffer, 16, 8);         /*elementary stream map length  is*/  
    /*audio*/  
//    bits_write(&bitsBuffer, 8, 0x90);       /*stream_type*/  
//    bits_write(&bitsBuffer, 8, 0xC0);       /*elementary_stream_id*/  
//    bits_write(&bitsBuffer, 16, 0);         /*elementary_stream_info_length is*/  
    /*video*/  
    bits_write(&bitsBuffer, 8, 0x1B);       /*stream_type*/  
    bits_write(&bitsBuffer, 8, 0xE0);       /*elementary_stream_id*/  
    bits_write(&bitsBuffer, 16, 0);         /*elementary_stream_info_length */  
    /*crc (2e b9 0f 3d)*/  
    bits_write(&bitsBuffer, 8, 0x45);       /*crc (24~31) bits*/  
    bits_write(&bitsBuffer, 8, 0xBD);       /*crc (16~23) bits*/  
    bits_write(&bitsBuffer, 8, 0xDC);       /*crc (8~15) bits*/  
    bits_write(&bitsBuffer, 8, 0xF4);       /*crc (0~7) bits*/  
    return 0;  
}  

/*** 
 *@remark:   pes头的封装,里面的具体数据的填写已经占位，可以参考标准 
 *@param :   pData      [in] 填充ps头数据的地址 
 *           stream_id  [in] 码流类型 
 *           paylaod_len[in] 负载长度 
 *           pts        [in] 时间戳 
 *           dts        [in] 
 *@return:   0 success, others failed 
*/  
int gb28181_make_pes_header(char *pData, int stream_id, int payload_len, unsigned long long pts, unsigned long long dts)  
{  
      
    bits_buffer_s   bitsBuffer;  
    bitsBuffer.i_size = PES_HDR_LEN;  
    bitsBuffer.i_data = 0;  
    bitsBuffer.i_mask = 0x80;  
    bitsBuffer.p_data = (unsigned char *)(pData);  
    memset(bitsBuffer.p_data, 0, PES_HDR_LEN);  
    /*system header*/  
    bits_write( &bitsBuffer, 24,0x000001);  /*start code*/  
    bits_write( &bitsBuffer, 8, (stream_id));   /*streamID*/  
    bits_write( &bitsBuffer, 16,(payload_len)+13);  /*packet_len*/ //指出pes分组中数据长度和该字节后的长度和  
 
    bits_write( &bitsBuffer, 2, 2 );        /*'10'*/  
    bits_write( &bitsBuffer, 2, 0 );        /*scrambling_control*/  
    bits_write( &bitsBuffer, 1, 0 );        /*priority*/  
    bits_write( &bitsBuffer, 1, 0 );        /*data_alignment_indicator*/  
    bits_write( &bitsBuffer, 1, 0 );        /*copyright*/  
    bits_write( &bitsBuffer, 1, 0 );        /*original_or_copy*/  

    bits_write( &bitsBuffer, 1, 1 );        /*PTS_flag*/  
    bits_write( &bitsBuffer, 1, 1 );        /*DTS_flag*/  
    bits_write( &bitsBuffer, 1, 0 );        /*ESCR_flag*/  
    bits_write( &bitsBuffer, 1, 0 );        /*ES_rate_flag*/  
    bits_write( &bitsBuffer, 1, 0 );        /*DSM_trick_mode_flag*/  
    bits_write( &bitsBuffer, 1, 0 );        /*additional_copy_info_flag*/  
    bits_write( &bitsBuffer, 1, 0 );        /*PES_CRC_flag*/  
    bits_write( &bitsBuffer, 1, 0 );        /*PES_extension_flag*/  

    bits_write( &bitsBuffer, 8, 10);        /*header_data_length*/   
    // 指出包含在 PES 分组标题中的可选字段和任何填充字节所占用的总字节数。该字段之前  
    //的字节指出了有无可选字段。  
    /*PTS,DTS*/   
	bits_write( &bitsBuffer, 4, 3 );                    /*'0011'*/  
    bits_write( &bitsBuffer, 3, ((pts)>>30)&0x07 );     /*PTS[32..30]*/  
    bits_write( &bitsBuffer, 1, 1 );  

    bits_write( &bitsBuffer, 15,((pts)>>15)&0x7FFF);    /*PTS[29..15]*/  
    bits_write( &bitsBuffer, 1, 1 );  

    bits_write( &bitsBuffer, 15,(pts)&0x7FFF);          /*PTS[14..0]*/  
    bits_write( &bitsBuffer, 1, 1 );  

    bits_write( &bitsBuffer, 4, 1 );                    /*'0001'*/  
    bits_write( &bitsBuffer, 3, ((dts)>>30)&0x07 );     /*DTS[32..30]*/  
    bits_write( &bitsBuffer, 1, 1 );  

    bits_write( &bitsBuffer, 15,((dts)>>15)&0x7FFF);    /*DTS[29..15]*/  
    bits_write( &bitsBuffer, 1, 1 );  

    bits_write( &bitsBuffer, 15,(dts)&0x7FFF);          /*DTS[14..0]*/  
    bits_write( &bitsBuffer, 1, 1 );  

    return 0;  
}  

int gb28181_make_psheader(Data_Info_s* pPacker)  
{  
    char szTempPacketHead[256] = {0};  
    int  nSizePos = 0;  
    // 1 package for ps header   
    gb28181_make_ps_header(szTempPacketHead + nSizePos, pPacker->s64CurPts);  
    nSizePos += PS_HDR_LEN;   
    //2 system header   
    if( pPacker->IFrame == 1 )  
    {  
        // 如果是I帧的话，则添加系统头  
        gb28181_make_sys_header(szTempPacketHead + nSizePos);  
        nSizePos += SYS_HDR_LEN;  
        //这个地方我是不管是I帧还是p帧都加上了map的，貌似只是I帧加也没有问题  
        gb28181_make_psm_header(szTempPacketHead + nSizePos);  
        nSizePos += PSM_HDR_LEN;
    }  
  
    ////////////todo output szTempPacketHead
  	fsps.write(szTempPacketHead, nSizePos);
    return 0;  
}

int gb28181_es2psForH264(char *pData, int nFrameLen, Data_Info_s* pPacker, int stream_type)  
{  
    char szTempPacketHead[256] = {0};  
    int  nSizePos = PES_HDR_LEN;  

    gb28181_make_psheader(pPacker);

    // 这里向后移动是为了方便拷贝pes头  
    //这里是为了减少后面音视频裸数据的大量拷贝浪费空间，所以这里就向后移动，在实际处理的时候，要注意地址是否越界以及覆盖等问题  
    while(nFrameLen > 0)  
    {  
        //每次帧的长度不要超过short类型，过了就得分片进循环行发送  
        int nSize = (nFrameLen > PS_PES_PAYLOAD_SIZE) ? PS_PES_PAYLOAD_SIZE : nFrameLen;  
        // 添加pes头  
        gb28181_make_pes_header(szTempPacketHead, stream_type ? 0xC0:0xE0, nSize, pPacker->s64CurPts, pPacker->s64CurPts);  

        ///////////////todo output szTempPacketHead and pData
	  	fsps.write(szTempPacketHead, nSizePos);
      	fsps.write(pData, nSize);

        //分片后每次发送的数据移动指针操作  
        nFrameLen -= nSize;  
        //这里也只移动nSize,因为在while向后移动的pes头长度，正好重新填充pes头数据  
        pData     += nSize;  
    }  
    return 0;  
}

//////////////////////////////////////////////////////////////////////////////////
//为NALU_t结构体分配内存空间  
NALU_t *AllocNALU(int buffersize)  
{  
    NALU_t *n;  
  
    if ((n = (NALU_t*)calloc (1, sizeof (NALU_t))) == NULL)  
    {  
        printf("AllocNALU: n");  
        exit(0);  
    }  
  
    n->max_size=buffersize;  
  
    if ((n->buf = (char*)calloc (buffersize, sizeof (char))) == NULL)  
    {  
        free (n);  
        printf ("AllocNALU: n->buf");  
        exit(0);  
    }  
  
    return n;  
}

//释放  
void FreeNALU(NALU_t *n)  
{  
    if (n)  
    {  
        if (n->buf)  
        {  
            free(n->buf);  
            n->buf=NULL;  
        }  
        free (n);  
    }  
}  
  
void OpenBitstreamFile (char *fn)  
{  
    if (NULL == (bits=fopen(fn, "rb")))  
    {  
        printf("open file error\n");  
        exit(0);  
    }  
}

//输出NALU长度和TYPE  
void dump(NALU_t *n)  
{  
    if (!n)return;  
    //printf("a new nal:");  
    printf(" len: %d  ", n->len);  
    printf("nal_unit_type: %x\n", n->nal_unit_type);  
}  

static int FindStartCode2 (unsigned char *Buf)  
{  
    if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=1) return 0; //判断是否为0x000001,如果是返回1  
    else return 1;  
}  
  
static int FindStartCode3 (unsigned char *Buf)  
{  
    if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=0 || Buf[3] !=1) return 0;//判断是否为0x00000001,如果是返回1  
    else return 1;  
}  

//这个函数输入为一个NAL结构体，主要功能为得到一个完整的NALU并保存在NALU_t的buf中，获取他的长度，填充F,IDC,TYPE位。  
//并且返回两个开始字符之间间隔的字节数，即包含有前缀的NALU的长度  
int GetAnnexbNALU (NALU_t *nalu)  
{  
    int pos = 0;  
    int StartCodeFound, rewind;  
    unsigned char *Buf;  
	static int info2=0, info3=0;  

    if ((Buf = (unsigned char*)calloc (nalu->max_size , sizeof(char))) == NULL)   
    {  
       printf ("GetAnnexbNALU: Could not allocate Buf memory\n");  
    }
  
    nalu->startcodeprefix_len=3;//初始化码流序列的开始字符为3个字节  
  
    if (3 != fread (Buf, 1, 3, bits))//从码流中读3个字节  
    {  
        free(Buf);  
        return 0;  
    }  
    info2 = FindStartCode2 (Buf);//判断是否为0x000001   
    if(info2 != 1)   
    {  
        //如果不是，再读一个字节  
        if(1 != fread(Buf+3, 1, 1, bits))//读一个字节  
        {  
            free(Buf);  
            return 0;  
        }  
        info3 = FindStartCode3 (Buf);//判断是否为0x00000001  
        if (info3 != 1)//如果不是，返回-1  
        {   
            free(Buf);  
            return -1;  
        }  
        else   
        {  
            //如果是0x00000001,得到开始前缀为4个字节  
            pos = 4;  
            nalu->startcodeprefix_len = 4;  
        }  
    }  
  
    else  
    {  
        //如果是0x000001,得到开始前缀为3个字节  
        nalu->startcodeprefix_len = 3;  
        pos = 3;  
    }  
  
    //查找下一个开始字符的标志位  
    StartCodeFound = 0;  
    info2 = 0;  
    info3 = 0;  
  
    while (!StartCodeFound)  
    {  
        if (feof (bits))//判断是否到了文件尾，文件结束，则返回非0值，否则返回0  
        {  
            nalu->len = (pos-1)-nalu->startcodeprefix_len;  //NALU单元的长度。  
            memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);       
            nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit  
            nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit  
            nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit  
            free(Buf);  
            return pos-1;  
        }  
        Buf[pos++] = fgetc (bits);//读一个字节到BUF中  
        info3 = FindStartCode3(&Buf[pos-4]);//判断是否为0x00000001  
        if(info3 != 1)  
        {  
           info2 = FindStartCode2(&Buf[pos-3]);//判断是否为0x000001  
        }  
              
        StartCodeFound = (info2 == 1 || info3 == 1);  
    }  
  
    // Here, we have found another start code (and read length of startcode bytes more than we should  
    // have.  Hence, go back in the file  
    rewind = (info3 == 1)? -4 : -3;  
  
    if (0 != fseek (bits, rewind, SEEK_CUR))//把文件指针指向前一个NALU的末尾，在当前文件指针位置上偏移 rewind。  
    {  
        free(Buf);  
        printf("GetAnnexbNALU: Cannot fseek in the bit stream file");  
    }  
  
    // Here the Start code, the complete NALU, and the next start code is in the Buf.    
    // The size of Buf is pos, pos+rewind are the number of bytes excluding the next  
    // start code, and (pos+rewind)-startcodeprefix_len is the size of the NALU excluding the start code  
  
    nalu->len = (pos+rewind)-nalu->startcodeprefix_len;    //NALU长度，不包括头部。  
    memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);//拷贝一个完整NALU，不拷贝起始前缀0x000001或0x00000001  
    nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit  
    nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit  
    nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit  
    free(Buf);  
  
    return (pos+rewind);//返回两个开始字符之间间隔的字节数，即包含有前缀的NALU的长度  
}  

/////////////////////////////////////////////////////////////////////////////////
char buffer[2 * 1000 * 1000 ] = {0};
int bufferlen = 0;

int main()
{
	fsps.open("./222.ps", std::ios::binary | std::ios::out);
//	fs264.open("./222.264", std::ios::binary | std::ios::out);

	OpenBitstreamFile((char*)"./111.264");//打开264文件，并将文件指针赋给bits,在此修改文件名实现打开别的264文件。  
	NALU_t *n = AllocNALU( 8 * 1000 * 1000 );//为结构体nalu_t及其成员buf分配空间。返回值为指向nalu_t存储空间的指针  
	Data_Info_s info;
	info.s64CurPts = 0;
	int sleeptime = 1000;
    char h264startcode[] = {0,0,0,1};

	while(!feof(bits))   
	{  
		int ret = GetAnnexbNALU(n);//每执行一次，文件的指针指向本次找到的NALU的末尾，下一个位置即为下个NALU的起始码0x000001  
		if( ret < 0 )
		{
			printf("break the loop\n");
			break;
		}
		printf("nalu length=%d %d %x %x %x %x \n", n->len, n->nal_unit_type, n->buf[0], n->buf[1], n->buf[2], n->buf[3]);
        info.s64CurPts += 3600;

        info.IFrame = 0;
		if( n->nal_unit_type == 7 )//|| n->nal_unit_type == 8 )
		{
//			info.IFrame = 1;
		}

        memcpy(buffer, h264startcode, sizeof(h264startcode));
        bufferlen = sizeof(h264startcode);
        memcpy(buffer+bufferlen, n->buf, n->len);
        bufferlen += n->len;

		gb28181_es2psForH264(buffer, bufferlen, &info, 0);
//		fs264.write(buffer, bufferlen);

//		usleep(sleeptime);
	}

	fsps.close();
//	fs264.close();
	fclose(bits);

	return 0;
}