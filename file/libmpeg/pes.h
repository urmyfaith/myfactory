#ifndef __PES_H__
#define __PES_H__

#ifdef WIN32
#include <basetsd.h>
typedef UINT8 uint8_t;
typedef UINT16 uint16_t;
typedef UINT32 uint32_t;
typedef UINT64 uint64_t;
#define __LITTLE_ENDIAN		1
#define __BIG_ENDIAN		2
#define __BYTE_ORDER		1
#else
#include <inttypes.h>
#include <endian.h>
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
#elif __BYTE_ORDER == __BIG_ENDIAN
#error "Temporarily not support big-endian systems."
#else
#error "Please fix <endian.h>"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************/
/* 实体流相关定义                                                       */
/************************************************************************/
#define STREAM_TYPE_VIDEO_MPEG1     0x01
#define STREAM_TYPE_VIDEO_MPEG2     0x02
#define STREAM_TYPE_AUDIO_MPEG1     0x03
#define STREAM_TYPE_AUDIO_MPEG2     0x04
#define STREAM_TYPE_PRIVATE_SECTION 0x05
#define STREAM_TYPE_PRIVATE_DATA    0x06
#define STREAM_TYPE_AUDIO_AAC       0x0f
#define STREAM_TYPE_AUDIO_AAC_LATM  0x11
#define STREAM_TYPE_VIDEO_MPEG4     0x10
#define STREAM_TYPE_VIDEO_H264      0x1b
#define STREAM_TYPE_VIDEO_HEVC      0x24
#define STREAM_TYPE_VIDEO_CAVS      0x42
#define STREAM_TYPE_VIDEO_VC1       0xea
#define STREAM_TYPE_VIDEO_DIRAC     0xd1
#define STREAM_TYPE_AUDIO_AC3       0x81
#define STREAM_TYPE_AUDIO_DTS       0x82
#define STREAM_TYPE_AUDIO_TRUEHD    0x83

typedef void (*SEGCALLBACK)(uint8_t *buf, int len, void *ctx);

#define MIN_PES_LENGTH	(1000)
#define MAX_PES_LENGTH	(65000)

// 实体流帧信息
typedef struct
{
	int program_number; // 节目编号，就是TsProgramInfo中prog数组下标，对于PS该值只能为0
	int stream_number;	// 流编号，就是TsProgramSpec中stream数组下标
	uint8_t *frame;		// 帧数据
	int length;			// 帧长度
	int is_key;			// 当前帧TS流化时是否带PAT和PMT
	uint64_t pts;		// 时间戳, 90kHz
	int ps_pes_length;	// 需要切分成PES的长度，该参数只对PS有效，最大不能超过MAX_PES_LENGTH
	SEGCALLBACK segcb;	// 对于PS，当生成一段数据（头部或PES）回调，不用可设为NULL
	void *ctx;			// 回调上下文
} TEsFrame;

// 判断是否视频
int lts_is_video(int type);
// 判断是否音频
int lts_is_audio(int type);

// 确定PES中的stream_id
uint8_t lts_pes_stream_id(int type, int program_number, int stream_number);

// 生成PES头部，返回头部总长度
int lts_pes_make_header(uint8_t stream_id, uint64_t pts, int es_len, uint8_t *dest, int maxlen);

// 解析PES头部长度，返回头部总长度
int lts_pes_parse_header(uint8_t *pes, int len, uint8_t *stream_id, uint64_t *pts, int *es_len);

#ifdef __cplusplus
}
#endif

#endif //__LITETS_TSSTREAM_H__
