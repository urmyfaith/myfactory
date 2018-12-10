#ifdef __cplusplus
extern "C"{
#endif
	#include "libavcodec/avcodec.h"  
	#include "libavformat/avformat.h"  
	#include "libswresample/swresample.h"  
	#include "libavutil/imgutils.h"  
#ifdef __cplusplus
}
#endif

#include <string>
#include "ffmpegresample.h"

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
};

void* resample_open(int in_channels,int in_sample_fmt,int in_sample_rate, int in_nb_samples)
{
	resampledesc_t * inst = new resampledesc_t;
	if( !inst )
		return NULL;

	SwrContext *pResampleCtx = swr_alloc();
	if (!pResampleCtx) 
		return NULL;

    int out_channel_layout = AV_CH_LAYOUT_STEREO;//输出声道  
    int out_sample_fmt = AV_SAMPLE_FMT_S16;//输出格式S16  
    int out_sample_rate = in_sample_rate;
    int out_nb_samples=in_nb_samples;//1024;  

    int out_channels =av_get_channel_layout_nb_channels(out_channel_layout);    
    inst->out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, (AVSampleFormat)out_sample_fmt,1);  
//    inst->out_buffer_size = av_samples_get_buffer_size(NULL, in_channels, out_nb_samples, (AVSampleFormat)out_sample_fmt,1);  
    inst->out_buffer = (uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE*2);  

	int in_channel_layout = av_get_default_channel_layout(in_channels);  
    pResampleCtx= swr_alloc_set_opts(pResampleCtx,out_channel_layout, (AVSampleFormat)out_sample_fmt, out_sample_rate,  
        in_channel_layout, (AVSampleFormat)in_sample_fmt, in_sample_rate,0,NULL);  

	if( swr_init(pResampleCtx) < 0){
		swr_free(&pResampleCtx);
		return NULL;
	}

	inst->in_channel_count = in_channel_layout;
	inst->in_bits_per_channel = in_sample_fmt;
	inst->in_samplerate = in_sample_rate;
	inst->pResampleCtx = pResampleCtx;

	return inst;
}

int resample_sound(void* handle, uint8_t **data, int linesize[], int samplecount, sound_resampled *frame)
{
	resampledesc_t * inst = (resampledesc_t*)handle;

	int out_samples = swr_convert(inst->pResampleCtx,&inst->out_buffer,MAX_AUDIO_FRAME_SIZE, (const uint8_t**)data, samplecount);  
	if (out_samples < 0) 
	{
		printf("resample_sound error:%u \n", out_samples);
		return -1;
	}

    frame->data = inst->out_buffer;
    frame->linesize = inst->out_buffer_size;

    return 0;
}

int resample_close(void* handle)
{
	resampledesc_t * inst = (resampledesc_t*)handle;
	av_free(inst->out_buffer);

	delete inst;

	return 0;
}
