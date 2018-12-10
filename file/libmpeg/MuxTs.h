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
#define MUXTS_ERROR_FILE_NOTTS -1	//�ļ�������.ts�ļ�
#define MUXTS_ERROR_FILE_NULL -2	//�ļ����ַ���ΪNULL
#define MUXTS_ERRTR_PAT_NOPROGRAM -3 //�Ҳ�����Ŀ�б�program
#define MUXTS_ERROR_FILLPAYLOAD_OVER -4 //�����ڴ��С
#define MUXTS_ERROR_OUTPUT_NOTFULL -5 //packet��δ����
#define MUXTS_ERRTR_PMT_NOSTREAM -6 //�Ҳ�����Ŀ���б�
#define MUXTS_ERRTR_CODEC_NOTUSE -7 //�����õı����ʽ

/**/
#define BYTE unsigned char
#define BYTE4 unsigned int
#define BYTE8 unsigned long long
#define uint64_t unsigned long long int
#define uint32_t unsigned int
#define uint8_t unsigned char

/*�����Ƿ����ô�С���ֽ���ת��*/
#define MUXTS_ENABLE_SWAP_ENDIAN32
#define MUXTS_ENABLE_SWAP_ENDIAN64
//����д��DTS��DTS��PTS��ͬ
#define MUXTS_ENABLE_WRITE_DTS

/**/
#define MUXTS_TS_PACKET_SIZE 188 //ÿ��ts����С188�ֽ�
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
	unsigned sync_byte : 8; //ͬ���ֽ�:0x47
	unsigned transport_error_indicator : 1; //������ʾ��Ϣ��1���ð�������1bit�������
	unsigned payload_unit_start_indicator : 1; //���ص�Ԫ��ʼ��־��packet����188�ֽ�ʱ��Ҫ���ص�ԪFF��䣩
	unsigned transport_priority : 1; //�������ȼ���־��1��ʾ�ߣ�
	unsigned PID : 13;	//ID�ţ�Ψһ�ĺ����Ӧ��ͬ�İ�
	unsigned transport_scrambling_control : 2; //���ܱ�־��00��ʾδ���ܣ�������ʾ�Ѽ���
	unsigned adaptation_field_control : 2;	//�����������
	unsigned continuity_counter : 4; //������������
};

struct TS_PAT_Program
{
	unsigned program_number : 16;  //��Ŀ��  
	unsigned reserved_3 : 3; // ����λ  
	unsigned program_map_PID : 13; // ��Ŀӳ����PID����Ŀ�Ŵ���0ʱ��Ӧ��PID��ÿ����Ŀ��Ӧһ��  
};

struct TS_PAT
{
	unsigned table_id : 8; //�̶�Ϊ0x00 ����־�Ǹñ���PAT��  
	unsigned section_syntax_indicator : 1; //���﷨��־λ���̶�Ϊ1  
	unsigned zero : 1; //0  
	unsigned reserved_1 : 2; // ����λ  
	unsigned section_length : 12; //��ʾ����һ���ֶο�ʼ��CRC32(��)֮�����õ��ֽ���  
	unsigned transport_stream_id : 16; //�ô�������ID��������һ��������������·���õ���  
	unsigned reserved_2 : 2;// ����λ  
	unsigned version_number : 5; //��Χ0-31����ʾPAT�İ汾��  
	unsigned current_next_indicator : 1; //���͵�PAT�ǵ�ǰ��Ч������һ��PAT��Ч  
	unsigned section_number : 8; //�ֶεĺ��롣PAT���ܷ�Ϊ��δ��䣬��һ��Ϊ00���Ժ�ÿ���ֶμ�1����������256���ֶ�  
	unsigned last_section_number : 8;  //���һ���ֶεĺ���  

	std::vector<TS_PAT_Program> program;
	
	unsigned CRC_32 : 32;  //CRC32У����  
};

struct TS_PMT_Stream
{
	unsigned stream_type : 8; //0x1b ��ʾ H264,0x0f ��ʾAAC,0x24��ʾhevc
	unsigned reserved_5 : 3; //0x07
	unsigned elementary_PID : 13; //����ָʾTS����PIDֵ����ЩTS��������صĽ�ĿԪ�� 
	unsigned reserved_6 : 4; //0x0F 
	unsigned ES_info_length : 12; //ǰ��λbitΪ00������ָʾ��������������ؽ�ĿԪ�ص�byte��    
	unsigned descriptor;
};

struct TS_PMT
{
	unsigned table_id : 8; //�̶�Ϊ0x02, ��ʾPMT��  
	unsigned section_syntax_indicator : 1; //�̶�Ϊ0x01  
	unsigned zero : 1; //0x01  
	unsigned reserved_1 : 2; //0x03  
	unsigned section_length : 12;//������λbit��Ϊ00����ָʾ�ε�byte�����ɶγ�����ʼ������CRC��  
	unsigned program_number : 16;// ָ���ý�Ŀ��Ӧ�ڿ�Ӧ�õ�Program map PID  
	unsigned reserved_2 : 2; //0x03  
	unsigned version_number : 5; //ָ��TS����Program map section�İ汾��  
	unsigned current_next_indicator : 1; //����λ��1ʱ����ǰ���͵�Program map section���ã�  
										 //����λ��0ʱ��ָʾ��ǰ���͵�Program map section�����ã���һ��TS����Program map section��Ч��  
	unsigned section_number : 8; //�̶�Ϊ0x00  
	unsigned last_section_number : 8; //�̶�Ϊ0x00  
	unsigned reserved_3 : 3; //0x07  
	unsigned PCR_PID : 13; //ָ��TS����PIDֵ����TS������PCR��  
						   //��PCRֵ��Ӧ���ɽ�Ŀ��ָ���Ķ�Ӧ��Ŀ��  
						   //�������˽���������Ľ�Ŀ������PCR�޹أ�������ֵ��Ϊ0x1FFF��  
	unsigned reserved_4 : 4; //Ԥ��Ϊ0x0F  
	unsigned program_info_length : 12; //ǰ��λbitΪ00������ָ���������Խ�Ŀ��Ϣ��������byte����  

	std::vector<TS_PMT_Stream> PMT_Stream;  //ÿ��Ԫ�ذ���8λ, ָʾ�ض�PID�Ľ�ĿԪ�ذ������͡��ô�PID��elementary PIDָ��  
	 
	unsigned CRC_32 : 32;
};

struct TS_PCR
{
	uint64_t program_clock_reference_base : 33; //����pts
	uint64_t reserved_1 : 6; //���1
	uint64_t program_clock_reference_extension : 9; //����0
};

//TS_ADAPTATION ��ʽ�ο� https://en.wikipedia.org/wiki/MPEG_transport_stream
struct TS_ADAPTATION
{
	unsigned adaptation_field_length : 8; //
	unsigned discontinuity_indicator : 1; //
	unsigned random_access_indicator : 1; //����Ϊ1
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
	unsigned head : 4; //Ĭ��0010B
	unsigned pts1 : 3; //pts 32��30λ
	unsigned resvered_1 : 1; //1
	unsigned pts2 : 15; //pts 29��15λ
	unsigned resvered_2 : 1; //1
	unsigned pts3 : 15; //pts 14��00λ
	unsigned resvered_3 : 1; //1
};
//DTS��ʽ��PTS��ͬ
struct TS_PES_DTS
{
	unsigned head : 4; //Ĭ��0010B
	unsigned pts1 : 3; //pts 32��30λ
	unsigned resvered_1 : 1; //1
	unsigned pts2 : 15; //pts 29��15λ
	unsigned resvered_2 : 1; //1
	unsigned pts3 : 15; //pts 14��00λ
	unsigned resvered_3 : 1; //1
};

//TS_PES ��ʽ�ο� https://en.wikipedia.org/wiki/Packetized_elementary_stream
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
	unsigned PMT_Quantity; //δʹ��
	//std::vector<INFO_PMT> pmt; //δʹ�ã�ֻʹ��һ��pmt
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

