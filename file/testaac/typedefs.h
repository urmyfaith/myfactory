#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__

#ifdef WIN32
#pragma pack(push) //保存对齐状态
#pragma pack(1)//设定为1字节对齐
#define __attribute__(x)
#endif
#include <inttypes.h>
//#include <endian.h>

// 每个节目的详情
#define MAX_STREAM_NUM		(4)

// 节目信息（目前最多支持1个节目2条流）
#define MAX_PROGRAM_NUM		(1)

/************************************************************************/
/* 节目信息定义                                                         */
/************************************************************************/
// 每条流的详情
typedef struct
{
	uint8_t type;			// [I]媒体类型
	uint8_t stream_id;		// [O]实体流ID（与PES头部id相同）
	int es_pid;				// [O]实体流的PID
	int continuity_counter;	// [O] TS包头部的连续计数器, 外部需要维护这个计数值, 必须每次传入上次传出的计数值
} TsStreamSpec;

typedef struct
{
	int stream_num;			// [I]这个节目包含的流个数
	int key_stream_id;		// {I]基准流编号
	int pmt_pid;			// [O]这个节目对应的PMT表的PID（TS解码用）
	int mux_rate;			// [O]这个节目的码率，单位为50字节每秒(PS编码用)
	TsStreamSpec stream[MAX_STREAM_NUM];
} TsProgramSpec;

typedef struct
{
	int program_num;		// [I]这个TS流包含的节目个数，对于PS该值只能为1
	int pat_pmt_counter;	// [O]PAT、PMT计数器
	TsProgramSpec prog[MAX_PROGRAM_NUM];
} TsProgramInfo;

/************************************************************************/
/* 接口函数，组帧。                                                     */
/************************************************************************/
typedef struct  
{
	TsProgramInfo info;		// 节目信息
	int is_pes;				// 属于数据，不是PSI
	int pid;				// 当前包的PID
	int program_no;			// 当前包所属的节目号
	int stream_no;			// 当前包所属的流号
	uint64_t pts;			// 当前包的时间戳
	uint64_t pes_pts;		// 当前PES的时间戳
	uint8_t *pack_ptr;		// 解出一包的首地址
	int pack_len;			// 解出一包的长度
	uint8_t *es_ptr;		// ES数据首地址
	int es_len;				// ES数据长度
	int pes_head_len;		// PES头部长度
	int sync_only;			// 只同步包，不解析包
	int ps_started;			// 已找到PS头部
} TDemux;

/************************************************************************/
/* little-endian                                                        */
/************************************************************************/
// PES Flags
typedef struct
{
	char orignal_or_copy: 1,
		 copyright: 1,
		 data_alignment_indicator: 1,
	 	 PES_priority: 1,
		 PES_scrambling_control: 2,
		 reserved: 2;
	char PES_extension_flag: 1,
		 PES_CRC_flag: 1,
		 additional_copy_info_flag: 1,
		 DSM_trick_mode_flag: 1,
		 ES_rate_flag: 1,
		 ESCR_flag: 1,
		 PTS_DTS_flags: 2;
} __attribute__((packed))
pes_head_flags;

// TS pure header
typedef struct
{
	char sync_byte;
	char PID_high5: 5,
		 transport_priority: 1,
		 payload_unit_start_indicator: 1,
		 transport_error_indicator: 1;
	char PID_low8;
	char continuity_counter: 4,
		 adaptation_field_control: 2,
		 transport_scrambling_control: 2;
} __attribute__((packed))
ts_pure_header;

// TS adaptation flags
typedef struct
{
	char adaptation_field_extension_flag: 1,
		 transport_private_data_flag: 1,
		 splicing_point_flag: 1,
		 OPCR_flag: 1,
		 PCR_flag: 1,
		 elementary_stream_priority_indicator: 1,
		 random_access_indicator: 1,
		 discontinuity_indicator: 1;
} __attribute__((packed))
ts_adaptation_flags;

// TS header
typedef struct
{
	ts_pure_header head;
	unsigned char adaptation_field_length;
	ts_adaptation_flags flags;
} __attribute__((packed))
ts_header;

// PAT
typedef struct
{
	char table_id;
	char section_length_high4: 4,
		 reserved1: 2,
		 zero: 1,
		 section_syntax_indicator: 1;
	char section_length_low8;
	char transport_stream_id_high8;
	char transport_stream_id_low8;
	char current_next_indicator: 1,
		 version_number: 5,
		 reserved2: 2;
	char section_number;
	char last_section_number;
} __attribute__((packed))
pat_section;

typedef struct
{
	char program_number_high8;
	char program_number_low8;
	char program_map_PID_high5: 5,
		 reserved: 3;
	char program_map_PID_low8;
} __attribute__((packed))
pat_map_array;

// PMT
typedef struct
{
	char table_id;
	char section_length_high4: 4,
		 reserved1: 2,
		 zero: 1,
		 section_syntax_indicator: 1;
	char section_length_low8;
	char program_number_high8;
	char program_number_low8;
	char current_next_indicator: 1,
		 version_number: 5,
		 reserved2: 2;
	char section_number;
	char last_section_number;
	char PCR_PID_high5: 5,
		 reserved3: 3;
	char PCR_PID_low8;
	char program_info_length_high4: 4,
		 reserved4: 4;
	char program_info_length_low8;
} __attribute__((packed))
pmt_section;

typedef struct
{
	char stream_type;
	char elementary_PID_high5: 5,
		 reserved1: 3;
	char elementary_PID_low8;
	char ES_info_length_high4: 4,
		 reserved2: 4;
	char ES_info_length_low8;
} __attribute__((packed))
pmt_stream_array;

/************************************************************************/
/* PS define:                                                           */
/************************************************************************/
typedef struct
{
	char start_code[4];				// == 0x000001BA
	char scr[6];
	char program_mux_rate[3];		// == 0x000003
	char pack_stuffing_length;		// == 0xF8
} __attribute__((packed))
ps_pack_header;

typedef struct  
{
	char start_code[4];				// == 0x000001BB
	char header_length[2];			// == 6 + 3 * stream_count(2)
	char rate_bound[3];				// == 0x800001
	char CSPS_flag: 1,				// == 0
		 fixed_flag: 1,				// == 0
		 audio_bound: 6;			// audio stream number
	char video_bound: 5,			// video stream number
		 marker_bit: 1,				// == 1
		 system_video_lock_flag: 1,	// == 1
		 system_audio_lock_flag: 1;	// == 1
	char reserved_byte;				// == 0xFF
} __attribute__((packed))
ps_system_header;

typedef struct
{
	// 0xB8 for all audio streams
	// 0xB9 for all video streams
	char stream_id;					
	char P_STD_buffer_size_bound_high5: 5,
		 // 0 for audio, scale x128B
		 // 1 for video, scale x1024B
		 P_STD_buffer_bound_scale: 1,
		 reserved: 2;				// == 3
	char P_STD_buffer_size_bound_low8;
} __attribute__((packed))
ps_system_header_stream_table;

// PSM
typedef struct
{
	char start_code[4];				// == 0x000001BC
	char header_length[2];			// == 6 + es_map_length
	char ps_map_version: 5,			// == 0
		 reserved1: 2,				// == 3
		 current_next_indicator: 1;	// == 1
	char marker_bit: 1,				// == 1
		 reserved2: 7;				// == 127
	char ps_info_length[2];			// == 0
	char es_map_length[2];			// == 4 * es_num
} __attribute__((packed))
ps_map;

typedef struct
{
	char stream_type;
	char es_id;
	char es_info_length[2];			// == 0
} __attribute__((packed))
ps_map_es;

#ifdef _MSC_VER
#pragma pack(pop)//恢复对齐状态
#endif

#endif //__LITETS_STREAMDEF_LE_H__
