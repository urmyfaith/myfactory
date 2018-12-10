#include "ffmpeg_demux.h"

struct ffmpegdemuxdesc_t
{
	AVFormatContext *pFormatCtx;
	AVPacket packet; 

 	AVCodecContext *pAudioCodecCtx;  
    AVFrame *pAudioFrame;  
    AVCodecContext *pVideoCodecCtx;  
    AVFrame *pVideoFrame;  
};

/////////////////////////////////////////////

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

AVCodecContext *AV_GetCodecContext(AVFormatContext *pFormatCtx,AVMediaType MediaType,int *iStreamID)
{
	if( !pFormatCtx )
		return NULL;
	
	AVCodecContext *pCodecCtx=NULL;
	for(unsigned int i=0; i<pFormatCtx->nb_streams; i++) 
	{
		AVStream *pStream=pFormatCtx->streams[i];
		if( pStream )
		{
			if( pStream->codec->codec_type==MediaType )
			{
				pCodecCtx=pFormatCtx->streams[i]->codec; 
				if( !pCodecCtx )
					return NULL;

				AVCodec *pCodec=avcodec_find_decoder(pCodecCtx->codec_id); 
				if( !pCodec ) 
					return NULL;
				if(avcodec_open2(pCodecCtx, pCodec,NULL)<0) 
					return NULL;

				*iStreamID=i;
			}
		}
	}

	return pCodecCtx;
}
/////////////////////////////////////////////////////

void* ffmpegdemux_open(const char* url)
{
	av_log_set_flags(AV_LOG_SKIP_REPEATED);
	avcodec_register_all();
#if CONFIG_AVDEVICE
	avdevice_register_all();
#endif
#if CONFIG_AVFILTER
	avfilter_register_all();
#endif
	av_register_all();
	avformat_network_init();

	//--------------------------open video source,get format infomation
	AVFormatContext *pFormatCtx = NULL;//avformat_alloc_context();
	int ret = avformat_open_input(&pFormatCtx, url, NULL,NULL);
	if( ret != 0 ) 
		return NULL;

	AVDictionary *options=NULL;
	if(avformat_find_stream_info(pFormatCtx,&options)<0) 
		return NULL;

	av_dump_format(pFormatCtx, 0, url, 0);

	ffmpegdemuxdesc_t *inst = new ffmpegdemuxdesc_t;
	if( !inst )
	{
		avformat_close_input(&pFormatCtx);
		return NULL;
	}
	inst->pFormatCtx = pFormatCtx;
    av_init_packet(&inst->packet);

    inst->pVideoFrame = inst->pAudioFrame = NULL;  

    int streamid = -1;
    inst->pVideoCodecCtx = AV_GetCodecContext(inst->pFormatCtx, AVMEDIA_TYPE_VIDEO, &streamid);
    if( inst->pVideoCodecCtx )
	    inst->pVideoFrame = av_frame_alloc();  

    inst->pAudioCodecCtx = AV_GetCodecContext(inst->pFormatCtx, AVMEDIA_TYPE_AUDIO, &streamid);
    if( inst->pAudioCodecCtx )
	    inst->pAudioFrame = av_frame_alloc();  

	return inst;
}

int ffmpegdemux_read(void* handle, ffmpegdemuxpacket_t *packet)
{
	ffmpegdemuxdesc_t *inst = (ffmpegdemuxdesc_t*)handle;

////////////////////////////////////////////////////////////////////////////////////
	if( av_read_frame(inst->pFormatCtx, &inst->packet) < 0 )
		return -1;

	packet->pts = inst->packet.pts;
	packet->data = inst->packet.data;
	packet->size = inst->packet.size;
	packet->stream_index = inst->packet.stream_index;

	packet->codec_id = inst->pFormatCtx->streams[packet->stream_index]->codec->codec_id;

	int codec_type = inst->pFormatCtx->streams[packet->stream_index]->codec->codec_type;
	if( codec_type == AVMEDIA_TYPE_VIDEO )
		packet->codec_type = 0;
	else if( codec_type == AVMEDIA_TYPE_AUDIO )
		packet->codec_type = 1;
	else
		packet->codec_type = 2;

	return 0;
}

double ffmpegdemux_getclock(void* handle, int streamid,int64_t pts)
{
	ffmpegdemuxdesc_t *inst = (ffmpegdemuxdesc_t*)handle;

	return av_q2d(inst->pFormatCtx->streams[streamid]->time_base) * pts;
}

int ffmpegdemux_decode(void *handle, int codecid, uint8_t *pBuffer, int dwBufsize, ffmpegdemuxframe_t *frame)
{
	ffmpegdemuxdesc_t *inst = (ffmpegdemuxdesc_t*)handle;

    AVPacket packet = {0};        
    av_init_packet(&packet);
    packet.data = pBuffer;//这里填入一个指向完整H264数据帧的指针  
    packet.size = dwBufsize;//这个填入H264数据帧的大小  

    if( inst->pVideoCodecCtx && inst->pVideoCodecCtx->codec_id == codecid )
    {
//        printf("ffmpegdec_decodev %d %d \n", inst->pVideoCodecCtx->codec_id, codecid);
        int ret = AV_DecodeVideo(inst->pVideoCodecCtx, inst->pVideoFrame, &packet);
        if( ret )
            return -1;

        frame->codec_type = 0;
        frame->data = inst->pVideoFrame->data;
        memcpy(frame->linesize, inst->pVideoFrame->linesize, sizeof(frame->linesize));
        frame->info.image.width = inst->pVideoCodecCtx->width;
        frame->info.image.height = inst->pVideoCodecCtx->height;
        frame->info.image.pix_fmt = inst->pVideoCodecCtx->pix_fmt;

//        printf("ffmpegdec_decodev %d %d \n", inst->pVideoCodecCtx->width, inst->pVideoCodecCtx->height);
    }
    else if( inst->pAudioCodecCtx && inst->pAudioCodecCtx->codec_id == codecid )
    {
        int ret = AV_DecodeAudio(inst->pAudioCodecCtx, inst->pAudioFrame, &packet);
        if( ret )
            return -1;

        frame->codec_type = 1;
        frame->data = inst->pAudioFrame->data;
        memcpy(frame->linesize, inst->pAudioFrame->linesize, sizeof(frame->linesize));
        frame->info.sample.nb_samples = inst->pAudioFrame->nb_samples;

        frame->info.sample.channels = inst->pAudioCodecCtx->channels;
        frame->info.sample.sample_fmt = inst->pAudioCodecCtx->sample_fmt;
        frame->info.sample.sample_rate = inst->pAudioCodecCtx->sample_rate;

//        printf("ffmpegdec_decodea %d %d \n", inst->pAudioCodecCtx->sample_rate, inst->pAudioCodecCtx->channels);
    }
    else
    {
        printf("ffmpegdemux_decode unknown packet type %d \n", codecid);
        return -1;
    }

    return 0;
}

int ffmpegdemux_close(void* handle)
{
	ffmpegdemuxdesc_t *inst = (ffmpegdemuxdesc_t*)handle;

	if( inst->pVideoCodecCtx )
	{
		avcodec_close(inst->pVideoCodecCtx);
		avcodec_free_context(&(inst->pVideoCodecCtx));
	}
    if( inst->pAudioCodecCtx )
    {
        avcodec_close(inst->pAudioCodecCtx);
        avcodec_free_context(&(inst->pAudioCodecCtx));
    }

	if( inst->pVideoFrame )
		av_frame_free(&(inst->pVideoFrame));
    if( inst->pAudioFrame )
        av_frame_free(&(inst->pAudioFrame));

	avformat_close_input(&inst->pFormatCtx);
	delete inst;

	return 0;
}
