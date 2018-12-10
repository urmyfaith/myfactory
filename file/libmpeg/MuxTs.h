/*************************
author:yu-s
date:2016.7.21
muxing h265/hevc,h264,aac to .ts
**************************/
#pragma once
#include <vector>
#include <fstream>
#include <bitset>
#include <cstring>

/*error code*/
#define MUXTS_ERROR_FILE_NOTTS -1	//文件名不是.ts文件
#define MUXTS_ERROR_FILE_NULL -2	//文件名字符串为NULL
#define MUXTS_ERRTR_PAT_NOPROGRAM -3 //找不到节目列表program
#define MUXTS_ERROR_FILLPAYLOAD_OVER -4 //超过内存大小
#define MUXTS_ERROR_OUTPUT_NOTFULL -5 //packet包未填满
#define MUXTS_ERRTR_PMT_NOSTREAM -6 //找不到节目流列表
#define MUXTS_ERRTR_CODEC_NOTUSE -7 //不可用的编码格式

/**/
#define BYTE unsigned char
#define BYTE4 unsigned int
#define BYTE8 unsigned long long
#define uint64_t unsigned long long int
#define uint32_t unsigned int
#define uint8_t unsigned char

/*定义是否启用大小端字节序转换*/
#define MUXTS_ENABLE_SWAP_ENDIAN32
#define MUXTS_ENABLE_SWAP_ENDIAN64
//启用写入DTS，DTS和PTS相同
#define MUXTS_ENABLE_WRITE_DTS

/**/
#define MUXTS_TS_PACKET_SIZE 188 //每个ts包大小188字节
enum AVCODEC
{
	MUXTS_CODEC_HEVC = 1,
	MUXTS_CODEC_H264,
	MUXTS_CODEC_AAC
};
#define MUXTS_CODEC_H265 MUXTS_CODEC_HEVC
#define MUXTS_TYPE_AUDIO 17
#define MUXTS_TYPE_VIDEO 18
#define MUXTS_PAT_SPACEPAT 42
/**/
#define BITSET(x,y,z) ((x <<= y)+(x |= z))
#define INCOUNT(x) ((++x)&=0x0f)

struct TS_HEADER
{
	unsigned sync_byte : 8; //同步字节:0x47
	unsigned transport_error_indicator : 1; //错误提示信息（1：该包有至少1bit传输错误）
	unsigned payload_unit_start_indicator : 1; //负载单元开始标志（packet不满188字节时需要负载单元FF填充）
	unsigned transport_priority : 1; //传输优先级标志（1表示高）
	unsigned PID : 13;	//ID号，唯一的号码对应不同的包
	unsigned transport_scrambling_control : 2; //加密标志，00表示未加密，其他表示已加密
	unsigned adaptation_field_control : 2;	//附加区域控制
	unsigned continuity_counter : 4; //包递增计数器
};

struct TS_PAT_Program
{
	unsigned program_number : 16;  //节目号  
	unsigned reserved_3 : 3; // 保留位  
	unsigned program_map_PID : 13; // 节目映射表的PID，节目号大于0时对应的PID，每个节目对应一个  
};

struct TS_PAT
{
	unsigned table_id : 8; //固定为0x00 ，标志是该表是PAT表  
	unsigned section_syntax_indicator : 1; //段语法标志位，固定为1  
	unsigned zero : 1; //0  
	unsigned reserved_1 : 2; // 保留位  
	unsigned section_length : 12; //表示从下一个字段开始到CRC32(含)之间有用的字节数  
	unsigned transport_stream_id : 16; //该传输流的ID，区别于一个网络中其它多路复用的流  
	unsigned reserved_2 : 2;// 保留位  
	unsigned version_number : 5; //范围0-31，表示PAT的版本号  
	unsigned current_next_indicator : 1; //发送的PAT是当前有效还是下一个PAT有效  
	unsigned section_number : 8; //分段的号码。PAT可能分为多段传输，第一段为00，以后每个分段加1，最多可能有256个分段  
	unsigned last_section_number : 8;  //最后一个分段的号码  

	std::vector<TS_PAT_Program> program;
	
	unsigned CRC_32 : 32;  //CRC32校验码  
};

struct TS_PMT_Stream
{
	unsigned stream_type : 8; //0x1b 表示 H264,0x0f 表示AAC,0x24表示hevc
	unsigned reserved_5 : 3; //0x07
	unsigned elementary_PID : 13; //该域指示TS包的PID值。这些TS包含有相关的节目元素 
	unsigned reserved_6 : 4; //0x0F 
	unsigned ES_info_length : 12; //前两位bit为00。该域指示跟随其后的描述相关节目元素的byte数    
	unsigned descriptor;
};

struct TS_PMT
{
	unsigned table_id : 8; //固定为0x02, 表示PMT表  
	unsigned section_syntax_indicator : 1; //固定为0x01  
	unsigned zero : 1; //0x01  
	unsigned reserved_1 : 2; //0x03  
	unsigned section_length : 12;//首先两位bit置为00，它指示段的byte数，由段长度域开始，包含CRC。  
	unsigned program_number : 16;// 指出该节目对应于可应用的Program map PID  
	unsigned reserved_2 : 2; //0x03  
	unsigned version_number : 5; //指出TS流中Program map section的版本号  
	unsigned current_next_indicator : 1; //当该位置1时，当前传送的Program map section可用；  
										 //当该位置0时，指示当前传送的Program map section不可用，下一个TS流的Program map section有效。  
	unsigned section_number : 8; //固定为0x00  
	unsigned last_section_number : 8; //固定为0x00  
	unsigned reserved_3 : 3; //0x07  
	unsigned PCR_PID : 13; //指明TS包的PID值，该TS包含有PCR域，  
						   //该PCR值对应于由节目号指定的对应节目。  
						   //如果对于私有数据流的节目定义与PCR无关，这个域的值将为0x1FFF。  
	unsigned reserved_4 : 4; //预留为0x0F  
	unsigned program_info_length : 12; //前两位bit为00。该域指出跟随其后对节目信息的描述的byte数。  

	std::vector<TS_PMT_Stream> PMT_Stream;  //每个元素包含8位, 指示特定PID的节目元素包的类型。该处PID由elementary PID指定  
	 
	unsigned CRC_32 : 32;
};

struct TS_PCR
{
	uint64_t program_clock_reference_base : 33; //等于pts
	uint64_t reserved_1 : 6; //填充1
	uint64_t program_clock_reference_extension : 9; //设置0
};

//TS_ADAPTATION 格式参考 https://en.wikipedia.org/wiki/MPEG_transport_stream
struct TS_ADAPTATION
{
	unsigned adaptation_field_length : 8; //
	unsigned discontinuity_indicator : 1; //
	unsigned random_access_indicator : 1; //设置为1
	unsigned elementary_stream_priority_indicator : 1; //
	unsigned PCR_flag : 1; //1
	unsigned OPCR_flag : 1; //
	unsigned splicing_point_flag : 1; //
	unsigned transport_private_data_flag : 1; //
	unsigned adaptation_field_extension_flag : 1; //
	TS_PCR PCR;
};

struct TS_PES_PTS
{
	unsigned head : 4; //默认0010B
	unsigned pts1 : 3; //pts 32到30位
	unsigned resvered_1 : 1; //1
	unsigned pts2 : 15; //pts 29到15位
	unsigned resvered_2 : 1; //1
	unsigned pts3 : 15; //pts 14到00位
	unsigned resvered_3 : 1; //1
};
//DTS格式和PTS相同
struct TS_PES_DTS
{
	unsigned head : 4; //默认0010B
	unsigned pts1 : 3; //pts 32到30位
	unsigned resvered_1 : 1; //1
	unsigned pts2 : 15; //pts 29到15位
	unsigned resvered_2 : 1; //1
	unsigned pts3 : 15; //pts 14到00位
	unsigned resvered_3 : 1; //1
};

//TS_PES 格式参考 https://en.wikipedia.org/wiki/Packetized_elementary_stream
struct TS_PES
{
	unsigned packet_start_code_prefix : 24;
	unsigned stream_id : 8;
	unsigned PESPacket_length : 16;
	unsigned marker_bits : 2; //10 binary or 0x02 hex
	unsigned PES_scrambling_control : 2;
	unsigned PES_priority : 1;
	unsigned data_alignment_indicator : 1;
	unsigned copyright : 1;
	unsigned original_or_copy : 1;
	unsigned PTS_DTS_flags : 2; //PTS DTS indicator	11 = both present, 01 is forbidden, 10 = only PTS, 00 = no PTS or DTS
	unsigned ESCR_flag : 1;
	unsigned ES_rate_flag : 1;
	unsigned DSM_trick_mode_flag : 1;
	unsigned additional_copy_info_flag : 1;
	unsigned PES_CRC_flag : 1;
	unsigned PES_extension_flag : 1;
	unsigned PES_header_data_length : 8;
};

struct TSData
{
	BYTE *buff;
	int seek;
};

struct INFO_PES
{
	unsigned PID;
	unsigned count;
	unsigned stream_type;
	unsigned stream_id;
	unsigned aorv; //type :MUXTS_TYPE_VIDEO or MUXTS_TYPE_AUDIO
};

struct INFO_PMT
{
	unsigned PID;
	unsigned count;
	unsigned PES_Quantity;
	unsigned program_number;
	std::vector<INFO_PES> pes;
};
struct INFO_PAT
{
	unsigned PID = 0;
	unsigned count;
	unsigned PMT_Quantity; //未使用
	//std::vector<INFO_PMT> pmt; //未使用，只使用一个pmt
	INFO_PMT pmt;
};
class MuxTs
{
public:
	MuxTs();
	~MuxTs();
public:
	int CreateFile(const char* filename);
	int CloseFile();
public:
	int AddNewProgram(unsigned PID, unsigned number);
	int AddNewStream(unsigned PID, AVCODEC codec);
	int WriteFrame(unsigned char * buffer, int len, uint64_t pts, AVCODEC type);
private:
	int WritePacketHeader(unsigned PID, unsigned payload_unit_start_indicator, unsigned adaptation_field_control, unsigned continuity_counter);
	int WritePAT(std::vector<TS_PAT_Program> &program, bool has_payload_unit_start_indicator);
	int WritePMT(std::vector<TS_PMT_Stream> PMT_Stream, int program_number, bool has_payload_unit_start_indicator);
	int WriteAdaptation(unsigned length, bool PCR_flag, TS_PCR pcr);
	int WritePES(unsigned length, unsigned stream_id, unsigned PTS_DTS_flag, unsigned data_length, TS_PES_PTS ts_pts);
private:
	int SwapBigLittleEndian(char *ptr, int bitlen);
	int SetCRC32(int begin);
	int FillPayload(int count);
	int Output2File();
private:
	int ret;
	std::ofstream m_ofile;
	TSData data;
	INFO_PAT m_pat;
	int pes_index_audio;
	int pes_index_video;
	unsigned m_packetCount;
	std::vector<TS_PAT_Program> _pmt;
	std::vector<TS_PMT_Stream> _pes;
};

