#ifdef __cplusplus
extern "C"{
#endif
	#include "libavcodec/avcodec.h"  
	#include "libavformat/avformat.h"  
	#include "libswresample/swresample.h"  
	#include "libavutil/imgutils.h"  
	#include <libavutil/opt.h>
#ifdef __cplusplus
}
#endif

#include <string>
#include "ffmpeg_resample.h"

#define MAX_AUDIO_FRAME_SIZE 19200  

struct resampledesc_t
{
	SwrContext *pResampleCtx;

	int in_channel_count;
	int in_bits_per_channel;
	int in_samplerate;

	uint8_t * out_buffer;
	int out_buffer_size;

	std::string buffer;
	AVFrame *audio_frame;
	AVFrame *src_audio_frame;
};

static AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
                                  uint64_t channel_layout,
                                  int sample_rate, int nb_samples)
{
    AVFrame *frame = av_frame_alloc();
    int ret;

    if (!frame) {
        fprintf(stderr, "Error allocating an audio frame\n");
        exit(1);
    }

    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

    if (nb_samples) {
        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            fprintf(stderr, "Error allocating an audio buffer\n");
            exit(1);
        }
    }

    return frame;
}
//////////////////////////////////////////////////////////////////////
void* resample_open(int in_channels,int in_sample_fmt,int in_sample_rate)
{
	resampledesc_t * inst = new resampledesc_t;
	if( !inst )
		return NULL;
/*
	SwrContext *pResampleCtx = swr_alloc();
	if (!pResampleCtx) 
	{
		delete inst;
		return NULL;
	}

    /* set options */
/*    
	av_opt_set_int       (pResampleCtx, "in_channel_count",   in_channels,       0);
    av_opt_set_int       (pResampleCtx, "in_sample_rate",     in_sample_rate,    0);
    av_opt_set_sample_fmt(pResampleCtx, "in_sample_fmt",      AV_SAMPLE_FMT_S16, 0);
    av_opt_set_int       (pResampleCtx, "out_channel_count",  in_channels,       0);
    av_opt_set_int       (pResampleCtx, "out_sample_rate",    in_sample_rate,    0);
    av_opt_set_sample_fmt(pResampleCtx, "out_sample_fmt",     AV_SAMPLE_FMT_FLTP,0);
*/
    /* initialize the resampling context */
/*    
	if ((swr_init(pResampleCtx)) < 0) {
		delete inst;
		swr_free(&pResampleCtx);
        fprintf(stderr, "Failed to initialize the resampling context\n");
		return NULL;
    }

    int out_sample_fmt = AV_SAMPLE_FMT_FLTP;//AV_SAMPLE_FMT_S16;//输出格式S16  
    int out_nb_samples=1024;  
    inst->out_buffer_size = av_samples_get_buffer_size(NULL, in_channels, out_nb_samples, 
    	(AVSampleFormat)out_sample_fmt,1);  

    inst->out_buffer = NULL;  
    inst->out_buffer = (uint8_t *)av_malloc(inst->out_buffer_size);  

    int dst_nb_samples = av_rescale_rnd(
	    	swr_get_delay(pResampleCtx, in_sample_rate) + out_nb_samples,
	        in_sample_rate, in_sample_rate, AV_ROUND_UP);
    printf("dst_nb_samples %d,inst->out_buffer %d out_buffer_size %d\n", 
    	dst_nb_samples, inst->out_buffer, inst->out_buffer_size);

	inst->pResampleCtx = pResampleCtx;
*/


	SwrContext *pResampleCtx = swr_alloc();
	if (!pResampleCtx) 
		return NULL;
	
	in_sample_fmt = AV_SAMPLE_FMT_S16;
    int out_channel_layout = AV_CH_LAYOUT_STEREO;//输出声道  
    int out_sample_fmt = AV_SAMPLE_FMT_FLTP;//AV_SAMPLE_FMT_S16;//输出格式S16  
    int out_sample_rate = in_sample_rate;
    int out_nb_samples=1024;  

    int out_channels =av_get_channel_layout_nb_channels(out_channel_layout);    
    inst->out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, 
    	(AVSampleFormat)out_sample_fmt,1);  
    inst->out_buffer = (uint8_t *)av_malloc(inst->out_buffer_size);  

	int in_channel_layout = av_get_default_channel_layout(in_channels);  
    pResampleCtx= swr_alloc_set_opts(pResampleCtx,out_channel_layout, (AVSampleFormat)out_sample_fmt, out_sample_rate,  
        in_channel_layout, (AVSampleFormat)in_sample_fmt, in_sample_rate,0,NULL);  

    printf("resample_open out_buffer_size %d\n", inst->out_buffer_size);
	if( swr_init(pResampleCtx) < 0){
		swr_free(&pResampleCtx);
		return NULL;
	}

	inst->in_channel_count = in_channel_layout;
	inst->in_bits_per_channel = in_sample_fmt;
	inst->in_samplerate = in_sample_rate;
	inst->pResampleCtx = pResampleCtx;

    inst->audio_frame = alloc_audio_frame(AV_SAMPLE_FMT_FLTP, AV_CH_LAYOUT_STEREO,
                                       in_sample_rate, out_nb_samples);
    inst->src_audio_frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, AV_CH_LAYOUT_STEREO,
                                       in_sample_rate, out_nb_samples);
	return inst;
}

int resample_sound(void* handle, uint8_t *data, int linesize, int samplecount, sound_resampled *frame)
{
	resampledesc_t * inst = (resampledesc_t*)handle;

//	int out_samples = swr_convert(inst->pResampleCtx,&inst->out_buffer,samplecount, 
//		(const uint8_t**)data, samplecount);  
//	int out_samples = swr_convert(inst->pResampleCtx, NULL,0, 
//		(const uint8_t**)data, samplecount);  
//	int out_samples = swr_convert(inst->pResampleCtx, inst->audio_frame->data,samplecount, 
//		(const uint8_t**)&data, samplecount);  

	memcpy(inst->src_audio_frame->data[0], data, linesize);
	int out_samples = swr_convert(inst->pResampleCtx, inst->audio_frame->data,samplecount, 
		(const uint8_t**)inst->src_audio_frame->data, samplecount);  
	if (out_samples < 0) 
	{
		printf("swr_convert error \n");
		return -1;
	}

//    frame->data = inst->out_buffer;
//    frame->linesize = inst->out_buffer_size;
    frame->data = inst->audio_frame->data[0];
    frame->linesize = inst->audio_frame->linesize[0];

    return 0;
}

int resample_close(void* handle)
{
	resampledesc_t * inst = (resampledesc_t*)handle;
	av_free(inst->out_buffer);

	delete inst;

	return 0;
}
