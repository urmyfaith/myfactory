#ifndef __FFMPEG_DEMUX_H__
#define __FFMPEG_DEMUX_H__

#ifdef __cplusplus
	#include <string>
extern "C"{
#endif
	#include "libavformat/avformat.h"
	#include "libavcodec/avcodec.h"
	#include "libswscale/swscale.h"
	#include "libswresample/swresample.h"
	#include "libavutil/avutil.h"
	#include "libavutil/opt.h"
	#include "libavfilter/avfilter.h"
	#include "libavdevice/avdevice.h"
	#include "libavutil/opt.h"
	#include "libavutil/error.h"
	#include "libavutil/mathematics.h"
	#include "libavutil/samplefmt.h"
#ifdef __cplusplus
}
#endif

struct ffmpegdemuxpacket_t
{
	uint8_t* data;
	int size;
	int64_t pts;
	int stream_index;
	int codec_id;
	int codec_type;//0-video 1-audio 2-unknown
};

struct ffmpegdemuxframe_t
{
	uint8_t** data;
	int linesize[8];
	int64_t pts;
	int stream_index;
	int codec_id;
	int codec_type;//0-video 1-audio 2-unknown
	union
	{
		struct image_t
		{
			int width;
			int height;
			int pix_fmt;		
		}image;
		struct sample_t
		{
			int channels;
			int sample_fmt;
			int sample_rate;	
			int nb_samples;	
		}sample;
	}info;
};

void* ffmpegdemux_open(const char* url);

int ffmpegdemux_read(void* handle, ffmpegdemuxpacket_t *packet);

double ffmpegdemux_getclock(void* handle, int streamid, int64_t pts);

int ffmpegdemux_decode(void *handle, int codecid, uint8_t *pBuffer, int dwBufsize, ffmpegdemuxframe_t *frame);

int ffmpegdemux_close(void* handle);

#endif