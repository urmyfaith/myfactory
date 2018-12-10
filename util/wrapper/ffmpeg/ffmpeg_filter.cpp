#include "ffmpeg_filter.h"
#include <stdio.h>
#include <string>

#ifdef  __cplusplus    
extern "C" {    
#endif    
    #include <libavcodec/avcodec.h>  
    #include <libavformat/avformat.h>  
    #include <libavutil/imgutils.h>
    #include <libavutil/avassert.h>
    #include <libavutil/channel_layout.h>
    #include <libavutil/opt.h>
    #include <libavutil/mathematics.h>
    #include <libavutil/avutil.h>

    #include "libavfilter/avfilter.h"
    #include "libavfilter/buffersink.h"
    #include "libavfilter/buffersrc.h"
#ifdef  __cplusplus    
}    
#endif

#define STREAM_FRAME_RATE 25 /* 25 images/s */

#define INPUT_SAMPLERATE     44100
#define INPUT_FORMAT         AV_SAMPLE_FMT_S16

struct ffmpeg_filter_tag_t
{
	std::string videoframe,audioframe;

    AVFilterGraph *filter_graph;

    AVFilterContext *audio_buffersrc_ctx;
    AVFilterContext *audio_buffersink_ctx;
    AVFilterContext *video_buffersrc_ctx;
    AVFilterContext *video_buffersink_ctx;

    AVFrame *video_input_frame;
    AVFrame *video_output_frame;

    AVFrame *audio_input_frame;
    AVFrame *audio_output_frame;

    uint8_t * audio_frame_buf;
    uint8_t *picture_buf;
};

static AVFrame *alloc_picture(int width, int height, AVPixelFormat pix_fmt)
{
    AVFrame *picture = av_frame_alloc();
    if (!picture)
        return NULL;

    picture->format = pix_fmt;
    picture->width  = width;
    picture->height = height;
    picture->pts = 0;

    int picture_size = avpicture_get_size(pix_fmt, width, height);  
    uint8_t *picture_buf = (uint8_t *)av_malloc(picture_size);  
    avpicture_fill((AVPicture *)picture, picture_buf, pix_fmt, width, height);

    /* allocate the buffers for the frame data */
/*    ret = av_frame_get_buffer(picture, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate frame data.\n");
        exit(1);
    }
*/
    return picture;
}
static AVFrame *alloc_audio_frame(int sample_rate, int channels, AVSampleFormat sample_fmt, int nb_samples)
{
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Error allocating an audio frame\n");
        return NULL;
    }

    frame->format = sample_fmt;
    frame->channels = channels;
    frame->channel_layout = av_get_default_channel_layout(channels);
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;
    frame->pts = 0;

    int size = av_samples_get_buffer_size(NULL, channels,
            nb_samples, sample_fmt, 1);
    uint8_t *audio_frame_buf = (uint8_t *)av_malloc(size);
    avcodec_fill_audio_frame(frame, channels, sample_fmt,
        (const uint8_t*)audio_frame_buf, size, 1);

/*
    if (nb_samples) {
        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            fprintf(stderr, "Error allocating an audio buffer\n");
            exit(1);
        }
    }
*/
    return frame;
}

static AVFilterContext* alloc_filter(AVFilterGraph *filter_graph, const char* filter_name, 
        const char* filter_alias, const char* filter_options)
{
    AVFilter *filer = avfilter_get_by_name(filter_name);
    if (!filer) {
        fprintf(stderr, "avfilter_get_by_name error.\n");
        return NULL;
    }

    AVFilterContext *filter_ctx = avfilter_graph_alloc_filter(filter_graph, filer, filter_alias);
    if (!filter_ctx) {
        fprintf(stderr, "avfilter_graph_alloc_filter error.\n");
        return NULL;
    }

    /* This filter takes no options. */
    int err = avfilter_init_str(filter_ctx, filter_options);
    if (err < 0) {
        fprintf(stderr, "avfilter_init_str error.\n");
        return NULL;
    }

    return filter_ctx;
}

static int link_filter(AVFilterContext *src_filter_ctx, AVFilterContext *dst_filter_ctx)
{
    int err = avfilter_link(src_filter_ctx, 0, dst_filter_ctx, 0);
    if (err < 0) {
        fprintf(stderr, "avfilter_link error\n");
        return -1;
    }

    return 0;
}
//////////////////////////////////////////////////////////////
void *ffmpeg_filter_alloc()
{
    av_register_all();
    avfilter_register_all();
    av_log_set_level(AV_LOG_DEBUG);
    
    AVFilterGraph *filter_graph = avfilter_graph_alloc();
    if( !filter_graph )
    {
        printf("avfilter_graph_alloc error\n");
        return NULL;
    }

	ffmpeg_filter_tag_t *inst = new ffmpeg_filter_tag_t;
    inst->audio_input_frame = inst->audio_output_frame = NULL;
    inst->video_input_frame = inst->video_output_frame = NULL;
    inst->audio_frame_buf = NULL;
    inst->filter_graph = filter_graph;

    return inst;
}

void* ffmpeg_filter_alloc_filter(void *handle, const char* filter_name, 
            const char* filter_alias, const char* filter_options)
{
    ffmpeg_filter_tag_t *inst = (ffmpeg_filter_tag_t*)handle;
    return alloc_filter(inst->filter_graph, filter_name, filter_alias, filter_options);
}

int ffmpeg_filter_link_filter(void *src_filter, void* dst_filter)
{
    return link_filter((AVFilterContext*)src_filter, (AVFilterContext*)dst_filter);
}

int ffmpeg_filter_set_video_source_filter(void *handle, void* source_filter)
{
    ffmpeg_filter_tag_t *inst = (ffmpeg_filter_tag_t*)handle;
    inst->video_buffersrc_ctx = (AVFilterContext*)source_filter;
    return 0;
}

int ffmpeg_filter_set_video_sink_filter(void *handle, void* sink_filter)
{
    ffmpeg_filter_tag_t *inst = (ffmpeg_filter_tag_t*)handle;

//    enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_RGB24, AV_PIX_FMT_NONE };

    enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
    av_opt_set_int_list((AVFilterContext*)sink_filter, "pix_fmts", pix_fmts,
                      AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);

    inst->video_buffersink_ctx = (AVFilterContext*)sink_filter;

    return 0;
}

int ffmpeg_filter_set_audio_source_filter(void *handle, void* source_filter)
{
    ffmpeg_filter_tag_t *inst = (ffmpeg_filter_tag_t*)handle;

    inst->audio_buffersrc_ctx = (AVFilterContext*)source_filter;

    return 0;
}

int ffmpeg_filter_set_audio_sink_filter(void *handle, void* sink_filter)
{
    ffmpeg_filter_tag_t *inst = (ffmpeg_filter_tag_t*)handle;
    inst->audio_buffersink_ctx = (AVFilterContext*)sink_filter;
    return 0;
}

int ffmpeg_filter_open(void *handle)
{
    ffmpeg_filter_tag_t *inst = (ffmpeg_filter_tag_t*)handle;

    if (avfilter_graph_config(inst->filter_graph, NULL) < 0)
        return-1;

    return 0;
}

int ffmpeg_filter_set_video(void *handle, int width, int height, int pixel_format)
{      
	ffmpeg_filter_tag_t *inst = (ffmpeg_filter_tag_t*)handle;

    printf("AV_PIX_FMT_YUV420P=%d AV_PIX_FMT_RGB24=%d \n", 
        AV_PIX_FMT_YUV420P, AV_PIX_FMT_RGB24);
    pixel_format = AV_PIX_FMT_YUV420P;
    inst->video_input_frame = alloc_picture(width, height, (AVPixelFormat)pixel_format);
    if( !inst->video_input_frame )
    {
        printf("alloc_picture failed \n");  
        return -1;        
    }
    inst->video_output_frame = av_frame_alloc();
    if( !inst->video_output_frame )
    {
        printf("alloc_picture failed \n");  
        return -1;        
    }

    return 0;
}

int ffmpeg_filter_set_audio(void *handle, int sample_rate, int channels, int sample_fmt, int nb_samples)
{
	ffmpeg_filter_tag_t *inst = (ffmpeg_filter_tag_t*)handle;

    sample_fmt = AV_SAMPLE_FMT_S16;
    inst->audio_input_frame = alloc_audio_frame(sample_rate, channels, (AVSampleFormat)sample_fmt, nb_samples);
    if( !inst->audio_input_frame )
    {
        printf("alloc_audio_frame failed \n");  
        return -1;  
    }
    inst->audio_output_frame = av_frame_alloc();
    if( !inst->audio_output_frame )
    {
        printf("alloc_audio_frame failed \n");  
        return -1;  
    }

    return 0;
}

int ffmpeg_filter_set_audio_data(void *handle, const char* data, int length)
{
    ffmpeg_filter_tag_t *inst = (ffmpeg_filter_tag_t*)handle;
    if( !inst->audio_input_frame )
        return -1;

    memcpy(inst->audio_input_frame->data[0],data, length);
    inst->audio_input_frame->pts++;

    int err = av_buffersrc_add_frame(inst->audio_buffersrc_ctx, inst->audio_input_frame);
    if( err < 0 )
        return -1;

    return 0;
}

int ffmpeg_filter_get_audio_data(void *handle, const char** data, int *length)
{
	ffmpeg_filter_tag_t *inst = (ffmpeg_filter_tag_t*)handle;
    if( !inst->audio_input_frame )
        return -1;

    int err = av_buffersink_get_frame(inst->audio_buffersink_ctx, inst->audio_output_frame);
    if( err < 0 )
        return -1;

    inst->audioframe.clear();
    inst->audioframe.append((const char*)inst->audio_output_frame->data[0], 
        inst->audio_output_frame->linesize[0]);

    av_frame_unref(inst->audio_output_frame);
    *data = inst->audioframe.data();
    *length = inst->audioframe.size();

    return 0;
}

int ffmpeg_filter_set_video_data(void *handle, const char* data, int length)
{
    ffmpeg_filter_tag_t *inst = (ffmpeg_filter_tag_t*)handle;

    memcpy(inst->video_input_frame->data[0],data, length);
    inst->video_input_frame->pts++;

   int err = av_buffersrc_add_frame(inst->video_buffersrc_ctx, inst->video_input_frame);
    if( err < 0 )
        return -1;

    return 0;
}

int ffmpeg_filter_get_video_data(void *handle, const char** data, int *length)
{
	ffmpeg_filter_tag_t *inst = (ffmpeg_filter_tag_t*)handle;

    int err = av_buffersink_get_frame(inst->video_buffersink_ctx, inst->video_output_frame);
    if( err < 0 )
        return -1;


    printf("ffmpeg_filter_get_video_data %d %d %d \n", 
            inst->video_output_frame->linesize[0],
            inst->video_output_frame->linesize[1],
            inst->video_output_frame->linesize[2]);


    inst->videoframe.clear();
    int width = inst->video_output_frame->width;
    int height = inst->video_output_frame->height;
    inst->videoframe.append((const char*)inst->video_output_frame->data[0], 
        width * height);
    inst->videoframe.append((const char*)inst->video_output_frame->data[1], 
        width * height / 4);
    inst->videoframe.append((const char*)inst->video_output_frame->data[2], 
        width * height / 4);

    av_frame_unref(inst->video_output_frame);

    *data = inst->videoframe.data();
    *length = inst->videoframe.size();

    return 0;

}

int ffmpeg_filter_destroy(void *handle)
{
	ffmpeg_filter_tag_t *inst = (ffmpeg_filter_tag_t*)handle;

    if( inst->filter_graph )
        avfilter_graph_free(&inst->filter_graph);

    if( inst->video_input_frame )
        av_frame_free(&inst->video_input_frame);
    if( inst->video_output_frame )
        av_frame_free(&inst->video_output_frame);
    if( inst->audio_input_frame )
        av_frame_free(&inst->audio_input_frame);
    if( inst->audio_output_frame )
        av_frame_free(&inst->audio_output_frame);

	delete inst;
	return 0;	
}

