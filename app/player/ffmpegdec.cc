#ifdef  __cplusplus    
extern "C" {    
#endif    
    #include <libavcodec/avcodec.h>  
    #include <libavformat/avformat.h>  
    #include <libswscale/swscale.h>  
    #include <libavutil/imgutils.h>
#ifdef  __cplusplus    
}    
#endif    
#include "ffmpegdec.h"

typedef struct 
{
 	AVCodecContext *pAudioCodecCtx;  
    AVFrame *pAudioFrame;  
    AVCodecContext *pVideoCodecCtx;  
    AVFrame *pVideoFrame;  

}ffmpegdeccontext_t;

static int AV_DecodeVideo(AVCodecContext *pCodecCtx, AVFrame *pFrameData, AVPacket *pPacket)
{
    if( !pCodecCtx || !pFrameData ||  !pPacket )
        return -1;

    int VideoDecodeFinished=0;
    avcodec_decode_video2(pCodecCtx, pFrameData, &VideoDecodeFinished, pPacket);
    if( VideoDecodeFinished )
    {
        return 0;
    }
    return -2;
}

static int AV_DecodeAudio(AVCodecContext *pCodecCtx,AVFrame *pFrameData,AVPacket *pPacket)
{
    if( !pCodecCtx || !pFrameData ||  !pPacket )
        return -1;

    int AudioDecodeFinished=0;
    avcodec_decode_audio4(pCodecCtx, pFrameData, &AudioDecodeFinished, pPacket);
    if( AudioDecodeFinished )
    {
        return 0;
    }
    return -2;
}

int AV_AllocCodecContext(int codecid, AVCodecContext **CodecCtx)
{
    /* find the video encoder */  
    AVCodec *pCodec = avcodec_find_decoder((AVCodecID)codecid);//AV_CODEC_ID_H264);        
    if (!pCodec)   
    {  
        printf("codec not found! \n");  
        return -1;  
    }  
      
    //初始化参数，下面的参数应该由具体的业务决定  
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);      
//    pCodecCtx->time_base.num = 1;  
//    pCodecCtx->frame_number = 1; //每包一个视频帧  
//    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;  
//    pCodecCtx->bit_rate = 0;  
//    pCodecCtx->time_base.den = 30;//帧率  
//    pCodecCtx->width = width;//视频宽  
//    pCodecCtx->height = height;//视频高  

    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0)  
    {
        printf("open codec failed \n");  
        return -1;  
    }

    *CodecCtx = pCodecCtx;
    return 0;
}

//////////////////////////////////////////////////////////////
void* ffmpegdec_open()
{
    //下面初始化h264解码库  
    av_register_all();  
    avcodec_register_all();  
    
    ffmpegdeccontext_t *obj = new ffmpegdeccontext_t;
    if( !obj )
    {
        printf("malloc object failed \n");  
        return NULL;  
    }

    obj->pVideoFrame = obj->pAudioFrame = NULL;  
    obj->pVideoCodecCtx = NULL;
    obj->pAudioCodecCtx = NULL;

    return obj;
}

int ffmpegdec_addaudiodec(void *handle, int audiocodecid)
{
    ffmpegdeccontext_t *inst = (ffmpegdeccontext_t*) handle;

    AVCodecContext *pAudioCodecCtx = NULL;
    if( AV_AllocCodecContext(audiocodecid, &pAudioCodecCtx) < 0 )
    {
        printf("AV_AllocCodecContext error \n");  
        return -1;          
    }

    if( !pAudioCodecCtx )
        return -1;          

    inst->pAudioFrame = av_frame_alloc();  
    inst->pAudioCodecCtx = pAudioCodecCtx;

    return 0;          
}

int ffmpegdec_addvideodec(void *handle, int video_codecid)
{
    ffmpegdeccontext_t *inst = (ffmpegdeccontext_t*) handle;

    AVCodecContext *pVideoCodecCtx = NULL;
    if( AV_AllocCodecContext(video_codecid, &pVideoCodecCtx) < 0 )
    {
        printf("AV_AllocCodecContext error \n");  
        return -1;          
    }

    if( !pVideoCodecCtx )
        return -1;          

    inst->pVideoFrame = av_frame_alloc();  
    inst->pVideoCodecCtx = pVideoCodecCtx;

    return 0;
}

int ffmpegdec_decode(void *handle, int codecid, uint8_t *pBuffer, int dwBufsize, ffmpegdecframe_t *frame)
{
    ffmpegdeccontext_t *obj = (ffmpegdeccontext_t*) handle;

    AVPacket packet = {0};        
    av_init_packet(&packet);
    packet.data = pBuffer;//这里填入一个指向完整H264数据帧的指针  
    packet.size = dwBufsize;//这个填入H264数据帧的大小  

    if( obj->pVideoCodecCtx && obj->pVideoCodecCtx->codec_id == codecid )
    {
//        printf("ffmpegdec_decodev %d %d \n", obj->pVideoCodecCtx->codec_id, codecid);
        int ret = AV_DecodeVideo(obj->pVideoCodecCtx, obj->pVideoFrame, &packet);
        if( ret )
            return -1;

        frame->codec_type = 0;
        frame->data = obj->pVideoFrame->data;
        memcpy(frame->linesize, obj->pVideoFrame->linesize, sizeof(frame->linesize));
        frame->info.image.width = obj->pVideoCodecCtx->width;
        frame->info.image.height = obj->pVideoCodecCtx->height;
        frame->info.image.pix_fmt = obj->pVideoCodecCtx->pix_fmt;

//        printf("ffmpegdec_decodev %d %d \n", obj->pVideoCodecCtx->width, obj->pVideoCodecCtx->height);
    }
    else if( obj->pAudioCodecCtx && obj->pAudioCodecCtx->codec_id == codecid )
    {
        int ret = AV_DecodeAudio(obj->pAudioCodecCtx, obj->pAudioFrame, &packet);
        if( ret )
            return -1;

        frame->codec_type = 1;
        frame->data = obj->pAudioFrame->data;
        memcpy(frame->linesize, obj->pAudioFrame->linesize, sizeof(frame->linesize));
        frame->info.sample.nb_samples = obj->pAudioFrame->nb_samples;

        frame->info.sample.channels = obj->pAudioCodecCtx->channels;
        frame->info.sample.sample_fmt = obj->pAudioCodecCtx->sample_fmt;
        frame->info.sample.sample_rate = obj->pAudioCodecCtx->sample_rate;

//        printf("ffmpegdec_decodea %d %d \n", obj->pAudioCodecCtx->sample_rate, obj->pAudioCodecCtx->channels);
    }
    else
    {
        printf("ffmpegdec_decode unknown packet type \n");
        return -1;
    }

    return 0;
}

int ffmpegdec_close(void* inst)
{
	if( !inst )
	{
		printf("decoder handle is null \n");
		return -1;
	}
	
	ffmpegdeccontext_t *obj = (ffmpegdeccontext_t*)inst;

	if( obj->pVideoCodecCtx )
	{
		avcodec_close(obj->pVideoCodecCtx);
		avcodec_free_context(&(obj->pVideoCodecCtx));
	}
    if( obj->pAudioCodecCtx )
    {
        avcodec_close(obj->pAudioCodecCtx);
        avcodec_free_context(&(obj->pAudioCodecCtx));
    }

	if( obj->pVideoFrame )
		av_frame_free(&(obj->pVideoFrame));
    if( obj->pAudioFrame )
        av_frame_free(&(obj->pAudioFrame));

	return 0;
}
