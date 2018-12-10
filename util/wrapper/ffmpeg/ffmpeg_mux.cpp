#include "ffmpeg_mux.h"
#include <stdio.h>
#include <string>

extern "C" {
#include <libavformat/avformat.h>
}
#define STREAM_FRAME_RATE 25 /* 25 images/s */

struct ffmpegmux_tag_t
{
    AVFormatContext *output_format_context;
    AVStream *video_st;
    AVStream *audio_st;

    std::string outputfilename;
};

void *ffmpegmux_alloc(const char* filenamewithsuffix)
{
    ffmpegmux_tag_t *inst = new ffmpegmux_tag_t;
	if( !inst )
		return NULL;

	inst->video_st = inst->audio_st = NULL;
	inst->outputfilename = filenamewithsuffix;

    av_register_all();
   	avformat_alloc_output_context2(&inst->output_format_context, NULL, NULL, filenamewithsuffix);

    return inst;
}

int ffmpegmux_addvideostream(void* handle, int width, int height)
{
	ffmpegmux_tag_t *inst = (ffmpegmux_tag_t*)handle;

    AVStream *output_stream = avformat_new_stream(inst->output_format_context, 0);
    if (!output_stream) {
        printf("Call avformat_new_stream function failed\n");
        return -1;
    }
    inst->video_st = output_stream;

    AVCodecContext *output_codec_context = output_stream->codec;
    output_codec_context->codec_type = AVMEDIA_TYPE_VIDEO;
    output_codec_context->codec_id = AV_CODEC_ID_H264;

    output_codec_context->width = width;
    output_codec_context->height = height;
    if (inst->output_format_context->oformat->flags & AVFMT_GLOBALHEADER) {
        inst->output_format_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    /* copy the stream parameters to the muxer */

    int ret = avcodec_parameters_from_context(output_stream->codecpar, output_stream->codec);
    if (ret < 0) {
        printf("Could not copy the stream parameters\n");
    }

    return 0;
}

int ffmpegmux_addaudiostream(void* handle, int sample_rate, int channels)
{
    ffmpegmux_tag_t *inst = (ffmpegmux_tag_t*)handle;

    AVStream *output_stream = avformat_new_stream(inst->output_format_context, 0);
    if (!output_stream) {
        printf("Call avformat_new_stream function failed\n");
        return -1;
    }
    inst->audio_st = output_stream;

    AVCodecContext *output_codec_context = output_stream->codec;
    output_codec_context->codec_type = AVMEDIA_TYPE_AUDIO;
    output_codec_context->codec_id = AV_CODEC_ID_AAC;

    output_codec_context->sample_rate = sample_rate;
    output_codec_context->channels = channels;
//    output_codec_context->channel_layout = av_get_default_channel_layout(output_codec_context->channels);
    output_codec_context->channel_layout = AV_CH_LAYOUT_STEREO;
    output_codec_context->sample_fmt = AV_SAMPLE_FMT_FLTP;

    if (inst->output_format_context->oformat->flags & AVFMT_GLOBALHEADER) {
        inst->output_format_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    /* copy the stream parameters to the muxer */

    int ret = avcodec_parameters_from_context(output_stream->codecpar, output_stream->codec);
    if (ret < 0) {
        printf("Could not copy the stream parameters\n");
    }

    return 0;
}

int ffmpegmux_open(void* handle)
{
	ffmpegmux_tag_t *inst = (ffmpegmux_tag_t*)handle;

    if (avio_open(&inst->output_format_context->pb, inst->outputfilename.c_str(), AVIO_FLAG_WRITE) < 0) {
        return -1;
    }

    AVDictionary *pAVDictionary = NULL;
    if (avformat_write_header(inst->output_format_context, &pAVDictionary)) {
        printf("Call avformat_write_header function failed.\n");
        return -1;
    }

    return 0;
}

int ffmpegmux_write_video_frame(void *handle, const char* frame, int length, uint64_t pts)
{
	ffmpegmux_tag_t *inst = (ffmpegmux_tag_t*)handle;

    AVPacket packet = {0};        
    av_init_packet(&packet);
    packet.stream_index = inst->video_st->index;
    packet.data = (uint8_t*)frame;//这里填入一个指向完整H264数据帧的指针  
    packet.size = length;//这个填入H264数据帧的大小  
    packet.pts = packet.dts = pts;

    int ret = av_interleaved_write_frame(inst->output_format_context, &packet);
    if (ret < 0) {
        printf("Call av_interleaved_write_frame function failed\n");
        return -1;
    }
    else if (ret > 0) {
        printf("End of stream requested\n");
        return -1;
    }

	return 0;
}

int ffmpegmux_write_audio_frame(void *handle, const char* frame, int length, uint64_t pts)
{
    ffmpegmux_tag_t *inst = (ffmpegmux_tag_t*)handle;

    AVPacket packet = {0};        
    av_init_packet(&packet);
    packet.stream_index = inst->audio_st->index;;
    packet.data = (uint8_t*)frame;//这里填入一个指向完整H264数据帧的指针  
    packet.size = length;//这个填入H264数据帧的大小  
    packet.pts = packet.dts = pts;

    int ret = av_interleaved_write_frame(inst->output_format_context, &packet);
    if (ret < 0) {
        printf("Call av_interleaved_write_frame function failed\n");
        return -1;
    }
    else if (ret > 0) {
        printf("End of stream requested\n");
        return -1;
    }

    return 0;
}

int ffmpegmux_destroy(void* handle)
{
	ffmpegmux_tag_t *inst = (ffmpegmux_tag_t*)handle;
    av_write_trailer(inst->output_format_context);
    
    if (!(inst->output_format_context->oformat->flags & AVFMT_NOFILE))
        /* Close the output file. */
        avio_closep(&inst->output_format_context->pb);

    /* free the stream */
    avformat_free_context(inst->output_format_context);

    delete inst;
	return 0;
}
