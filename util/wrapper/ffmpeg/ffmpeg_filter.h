#ifndef __FFMPEG_FILTER_H__
#define __FFMPEG_FILTER_H__
#include <stdint.h>

void *ffmpeg_filter_alloc();

void* ffmpeg_filter_alloc_filter(void *handle, const char* filter_name, 
        	const char* filter_alias, const char* filter_options);
int ffmpeg_filter_link_filter(void *src_filter, void* dst_filter);

int ffmpeg_filter_set_video_source_filter(void *handle, void* source_filter);
int ffmpeg_filter_set_video_sink_filter(void *handle, void* sink_filter);
int ffmpeg_filter_set_audio_source_filter(void *handle, void* source_filter);
int ffmpeg_filter_set_audio_sink_filter(void *handle, void* sink_filter);

int ffmpeg_filter_open(void *handle);

int ffmpeg_filter_set_video(void *handle, int width, int height, int pixel_format);
int ffmpeg_filter_set_audio(void *handle, int sample_rate, int channels, 
	int sample_fmt, int nb_samples);

int ffmpeg_filter_set_audio_data(void *handle, const char* data, int length);
int ffmpeg_filter_get_audio_data(void *handle, const char** data, int *length);

int ffmpeg_filter_set_video_data(void *handle, const char* data, int length);
int ffmpeg_filter_get_video_data(void *handle, const char** data, int *length);

int ffmpeg_filter_destroy(void *handle);

#endif