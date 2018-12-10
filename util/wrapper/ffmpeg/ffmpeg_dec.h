#ifndef __FFMPEG_DEC_H__
#define __FFMPEG_DEC_H__

struct ffmpegdecframe_t
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

void* ffmpegdec_open();

int ffmpegdec_addaudiodec(void *handle, int audiocodecid);
int ffmpegdec_addvideodec(void *handle, int video_codecid);

int ffmpegdec_decode(void *handle, int codecid, uint8_t *pBuffer, int dwBufsize, ffmpegdecframe_t *frame);

int ffmpegdec_close(void* inst);

#endif