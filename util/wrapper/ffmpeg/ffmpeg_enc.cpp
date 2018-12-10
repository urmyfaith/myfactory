#include "ffmpeg_enc.h"
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
#ifdef  __cplusplus    
}    
#endif

#define STREAM_FRAME_RATE 25 /* 25 images/s */

struct ffmpeg_enc_tag_t
{
	std::string videoframe,audioframe;
 	AVCodecContext *pCodecCtx;
 	AVCodec *pCodec;
    AVFrame *video_frame;
    AVFrame *audio_frame;

    uint8_t * audio_frame_buf;
    uint8_t *picture_buf;
};

static AVFrame *alloc_picture(AVCodecContext *pCodecCtx)
{
    AVFrame *picture = av_frame_alloc();
    if (!picture)
        return NULL;

    picture->format = pCodecCtx->pix_fmt;
    picture->width  = pCodecCtx->width;
    picture->height = pCodecCtx->height;
    picture->pts = 0;

    int picture_size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);  
    uint8_t *picture_buf = (uint8_t *)av_malloc(picture_size);  
    avpicture_fill((AVPicture *)picture, picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);

    /* allocate the buffers for the frame data */
/*    ret = av_frame_get_buffer(picture, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate frame data.\n");
        exit(1);
    }
*/
    return picture;
}
static AVFrame *alloc_audio_frame(AVCodecContext *pCodecCtx, int nb_samples)
{
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Error allocating an audio frame\n");
        return NULL;
    }

    frame->format = pCodecCtx->sample_fmt;
    frame->channel_layout = pCodecCtx->channel_layout;
    frame->sample_rate = pCodecCtx->sample_rate;
    frame->nb_samples = nb_samples;
    frame->pts = 0;

    int size = av_samples_get_buffer_size(NULL, pCodecCtx->channels,
            pCodecCtx->frame_size, pCodecCtx->sample_fmt, 1);
    uint8_t *audio_frame_buf = (uint8_t *)av_malloc(size);
    avcodec_fill_audio_frame(frame, pCodecCtx->channels, pCodecCtx->sample_fmt,
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
//////////////////////////////////////////////////////////////
void *ffmpeg_enc_alloc()
{
    av_register_all();
    av_log_set_level(AV_LOG_DEBUG);
    
	ffmpeg_enc_tag_t *inst = new ffmpeg_enc_tag_t;
	inst->pCodecCtx = NULL;
	inst->pCodec = NULL;
    inst->audio_frame = NULL;
    inst->video_frame = NULL;
    inst->audio_frame_buf = NULL;
    return inst;
}

int ffmpeg_enc_set_video(void *handle, const char* codecname, int width, int height, int pixel_format)
{      
	ffmpeg_enc_tag_t *inst = (ffmpeg_enc_tag_t*)handle;

    AVCodec* pCodec = avcodec_find_encoder_by_name(codecname);
//    codecid = AV_CODEC_ID_H264;
//    AVCodec *pCodec = avcodec_find_encoder((AVCodecID)codecid);//AV_CODEC_ID_H264);        
    if (!pCodec)   
    {  
        printf("codec not found! \n");  
        return NULL;  
    }  
      
    //初始化参数，下面的参数应该由具体的业务决定  
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);    
    inst->pCodecCtx = pCodecCtx;
    inst->pCodec = pCodec;

	//初始化参数，下面的参数应该由具体的业务决定  
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;  
    pCodecCtx->codec_id = AV_CODEC_ID_H264;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;  
    pCodecCtx->width = width;//视频宽  
    pCodecCtx->height = height;//视频高  

    pCodecCtx->time_base       = (AVRational){ 1, STREAM_FRAME_RATE };

    if(avcodec_open2(pCodecCtx, inst->pCodec, NULL) < 0)  
    {
        printf("open codec failed \n");  
        return -1;  
    }

    inst->video_frame = alloc_picture(pCodecCtx);
    if( !inst->video_frame )
    {
        printf("alloc_picture failed \n");  
        return -1;        
    }

    return 0;
}

int ffmpeg_enc_set_audio(void *handle, const char* codecname, int sample_rate, int nb_samples, int channels)
{
	ffmpeg_enc_tag_t *inst = (ffmpeg_enc_tag_t*)handle;

    AVCodec* pCodec = avcodec_find_encoder_by_name(codecname);
//    codecid = AV_CODEC_ID_AAC;
//    AVCodec *pCodec = avcodec_find_encoder((AVCodecID)codecid);//AV_CODEC_ID_H264);        
    if (!pCodec)   
    {  
        printf("codec not found! \n");  
        return NULL;  
    }  
      
    //初始化参数，下面的参数应该由具体的业务决定  
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);   
    avcodec_get_context_defaults3(pCodecCtx, pCodec);
    inst->pCodecCtx = pCodecCtx;
    inst->pCodec = pCodec;

    //初始化参数，下面的参数应该由具体的业务决定  
    pCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;  
    pCodecCtx->codec_id = AV_CODEC_ID_AAC;
    pCodecCtx->bit_rate = 64000;  
    pCodecCtx->sample_rate = sample_rate;
    pCodecCtx->channels = channels;
    pCodecCtx->channel_layout = av_get_default_channel_layout(pCodecCtx->channels);
    pCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;//AV_SAMPLE_FMT_FLTP;

    if(avcodec_open2(pCodecCtx, inst->pCodec, NULL) < 0)  
    {
        printf("open codec failed \n");  
        return -1;  
    }
    
    inst->audio_frame = alloc_audio_frame(pCodecCtx, nb_samples);
    if( !inst->audio_frame )
    {
        printf("alloc_audio_frame failed \n");  
        return -1;  
    }
    return 0;
}

int ffmpeg_enc_encode_audio(void *handle, const char* framedata, int length, 
            const char **packetdata, int *packetlength)
{
	ffmpeg_enc_tag_t *inst = (ffmpeg_enc_tag_t*)handle;
    if( !inst->audio_frame )
        return -1;

    memcpy(inst->audio_frame->data[0],framedata, length);
    inst->audio_frame->pts++;

    AVPacket pkt = { 0 };
    av_init_packet(&pkt);
    int got_packet;
    int ret = avcodec_encode_audio2(inst->pCodecCtx, &pkt, inst->audio_frame, &got_packet);
    if (ret < 0) {
        printf("Error encoding audio frame: %d\n", ret);
        return -1;
    }
    printf("avcodec_encode_audio2 %d %d %d %d %d\n", inst->audio_frame->linesize[0],inst->audio_frame->linesize[1], 
        length, pkt.size, got_packet);

    if( !got_packet )
        return -2;

    inst->audioframe.clear();
    inst->audioframe.append((const char*)pkt.data, pkt.size);
    av_free_packet(&pkt);

    *packetdata = inst->audioframe.data();
    *packetlength = inst->audioframe.size();

    return 0;
}

int ffmpeg_enc_encode_video(void *handle, const char* framedata, int length, 
            const char **packetdata, int *packetlength)
{
	ffmpeg_enc_tag_t *inst = (ffmpeg_enc_tag_t*)handle;

    memcpy(inst->video_frame->data[0],framedata, length);
    inst->video_frame->pts++;

    AVPacket pkt = { 0 };
    av_init_packet(&pkt);

    int got_packet;
    int ret = avcodec_encode_video2(inst->pCodecCtx, &pkt, inst->video_frame, &got_packet);
    if (ret < 0) {
        printf("Error encoding video frame: %d\n", ret);
        return -1;
    }

    if( !got_packet )
        return -2;

    inst->videoframe.clear();
    inst->videoframe.append((const char*)pkt.data, pkt.size);

    *packetdata = inst->videoframe.data();
    *packetlength = inst->videoframe.size();
    av_free_packet(&pkt);

    return 0;

}

int ffmpeg_enc_destroy(void *handle)
{
	ffmpeg_enc_tag_t *inst = (ffmpeg_enc_tag_t*)handle;
	if( inst->pCodecCtx )
	{
		avcodec_close(inst->pCodecCtx);
		avcodec_free_context(&(inst->pCodecCtx));
	}

    if( inst->video_frame )
        av_frame_free(&inst->video_frame);
    if( inst->audio_frame )
        av_frame_free(&inst->audio_frame);
	delete inst;
	return 0;	
}

