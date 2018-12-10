#include <fstream>
#include <string>
#include <unistd.h>

#include "audio_generator.h"
#include "video_generator.h"
#include "ffmpeg_enc.h"
#include "ffmpegresample.h"
#include "ffmpeg_mux.h"
#include "ffmpeg_filter.h"

//ffplay -ar 16000 -channels 1 -f s16le -i xxx.pcm
int testaac3()
{
	void* handle = audio_generator_alloc(44100, 16, 2, 1024);
	std::fstream fs;
	fs.open("1.pcm", std::ios::binary|std::ios::out);

	while( 1 )
	{
		const char *frame = NULL;
		int length = 0;
		audio_generator_get_audio_frame(handle, &frame, &length);
		if( length > 0 )
		{
			fs.write(frame,length);
		}

		usleep(100 * 1000);
	}

	return 0;
}

//ffplay -f rawvideo -video_size 1280x720 xxx.yuv
int testaac4()
{
	void* handle = video_generator_alloc(960, 360, 1);
//	void* handle = video_generator_alloc(352, 288, 1);
	std::fstream fs;
	fs.open("1.yuv", std::ios::binary|std::ios::out);

	while( 1 )
	{
		const char *frame = NULL;
		int length = 0;
		video_generator_get_yuv420p_frame(handle, &frame, &length);
		if( length > 0 )
		{
			fs.write(frame,length);
		}

		usleep(100 * 1000);
	}

	return 0;
}

int testaac5()
{
	void* handle = audio_generator_alloc(44100, 16, 2, 1024);
	void *resamplehandle = resample_open(2, 1, 64000);

	void* enchandle = ffmpeg_enc_alloc();
	ffmpeg_enc_set_audio(enchandle, "libfdk_aac", 44100, 1024, 2);

	std::fstream fs;
	fs.open("1.aac", std::ios::binary|std::ios::out);

	sound_resampled resample;	
	while( 1 )
	{
		const char *frame = NULL;
		int length = 0;
		audio_generator_get_audio_frame(handle, &frame, &length);
		if( length > 0 )
		{
			const char *packet = NULL;
			int packetlength = 0;
			ffmpeg_enc_encode_audio(enchandle, (const char*)frame, length, &packet, &packetlength);
			if( packetlength > 0 )
				fs.write(packet,packetlength);				
		}

		usleep(100 * 1000);
	}

	return 0;
}

int testaac6()
{
	void* handle = video_generator_alloc(352, 288, 1);

	void* enchandle = ffmpeg_enc_alloc();
	ffmpeg_enc_set_video(enchandle, "libx264", 352, 288, 2);

	std::fstream fs;
	fs.open("1.h264", std::ios::binary|std::ios::out);

	sound_resampled resample;	
	while( 1 )
	{
		const char *frame = NULL;
		int length = 0;
		video_generator_get_yuv420p_frame(handle, &frame, &length);
		if( length > 0 )
		{
			const char *packet = NULL;
			int packetlength = 0;
			ffmpeg_enc_encode_video(enchandle, frame, length, &packet, &packetlength);
			if( packetlength > 0 )
				fs.write(packet,packetlength);				
		}

		usleep(100 * 1000);
	}

	return 0;
}

int testaac7()
{
	void* filterhandle = ffmpeg_filter_alloc();
	void *filter1 = ffmpeg_filter_alloc_filter(filterhandle, "testsrc", "testsrc", NULL);
	if( !filter1 )
	{
		printf("alloc testsrc error \n");
		return -1;
	}
	void *filter2 = ffmpeg_filter_alloc_filter(filterhandle, "scale", "scale", "960:540");
	ffmpeg_filter_link_filter(filter1, filter2);

	void *filter3 = ffmpeg_filter_alloc_filter(filterhandle, "buffersink", "buffersink", NULL);
	ffmpeg_filter_set_video_sink_filter(filterhandle, filter3);
	ffmpeg_filter_link_filter(filter2, filter3);

	ffmpeg_filter_open(filterhandle);
	ffmpeg_filter_set_video(filterhandle, 320, 240, 1);
	
	std::fstream fs;
	fs.open("1.yuv", std::ios::binary|std::ios::out);

	while( 1 )
	{
		const char *frame = NULL;
		int length = 0;
//		printf("ffmpeg_filter_get_video_data \n");
		if( ffmpeg_filter_get_video_data(filterhandle, &frame, &length) >= 0 )
		{
			if( length > 0 )
			{
				printf("frame length = %d \n", length);
				fs.write(frame,length);				
			}			
		}

		usleep(100 * 1000);
	}

	return 0;
}

int testaac8()
{
	void* filterhandle = ffmpeg_filter_alloc();
	void *filter1 = ffmpeg_filter_alloc_filter(filterhandle, "anoisesrc", "anoisesrc", NULL);
	if( !filter1 )
	{
		printf("alloc anoisesrc error \n");
		return -1;
	}

	void *filter3 = ffmpeg_filter_alloc_filter(filterhandle, "abuffersink", "abuffersink", NULL);
	ffmpeg_filter_set_audio_sink_filter(filterhandle, filter3);
	ffmpeg_filter_link_filter(filter1, filter3);

	ffmpeg_filter_open(filterhandle);
	ffmpeg_filter_set_audio(filterhandle, 44100, 2, 1, 1024);
	
	std::fstream fs;
	fs.open("1.pcm", std::ios::binary|std::ios::out);

	while( 1 )
	{
		const char *frame = NULL;
		int length = 0;
//		printf("ffmpeg_filter_get_video_data \n");
		if( ffmpeg_filter_get_audio_data(filterhandle, &frame, &length) >= 0 )
		{
			if( length > 0 )
			{
				printf("frame length = %d \n", length);
				fs.write(frame,length);				
			}			
		}

		usleep(100 * 1000);
	}

	return 0;
}

int testaac9()
{
	void* handle = audio_generator_alloc(44100, 16, 2, 1024);

	void* filterhandle = ffmpeg_filter_alloc();
	std::string abufferoptions = "sample_fmt=s16:channels=2:sample_rate=44100:channel_layout=stereo";
	void *filter1 = ffmpeg_filter_alloc_filter(filterhandle, "abuffer", "abuffer", abufferoptions.c_str());
	if( !filter1 )
	{
		printf("alloc abuffer error \n");
		return -1;
	}

	ffmpeg_filter_set_audio_source_filter(filterhandle, filter1);
	void *filter3 = ffmpeg_filter_alloc_filter(filterhandle, "abuffersink", "abuffersink", NULL);
	ffmpeg_filter_set_audio_sink_filter(filterhandle, filter3);
	ffmpeg_filter_link_filter(filter1, filter3);

	ffmpeg_filter_open(filterhandle);
	ffmpeg_filter_set_audio(filterhandle, 44100, 2, 16, 1024);
	
	std::fstream fs;
	fs.open("1.pcm", std::ios::binary|std::ios::out);

	while( 1 )
	{
		const char *frame = NULL;
		int length = 0;
		audio_generator_get_audio_frame(handle, &frame, &length);

		printf("ffmpeg_filter_set_audio_data %d \n", length);
		ffmpeg_filter_set_audio_data(filterhandle, frame, length);

		printf("ffmpeg_filter_get_audio_data \n");
		if( ffmpeg_filter_get_audio_data(filterhandle, &frame, &length) >= 0 )
		{
			if( length > 0 )
			{
				printf("frame length = %d \n", length);
				fs.write(frame,length);				
			}			
		}

		usleep(100 * 1000);
	}

	return 0;
}

int testaac10()
{
	void* handle = video_generator_alloc(960, 540, 1);

	void* filterhandle = ffmpeg_filter_alloc();
	std::string bufferoptions = "video_size=960x540:pix_fmt=0:time_base=25/1";
	void *filter1 = ffmpeg_filter_alloc_filter(filterhandle, "buffer", "buffer", bufferoptions.c_str());
	if( !filter1 )
	{
		printf("alloc buffer error \n");
		return -1;
	}

	ffmpeg_filter_set_video_source_filter(filterhandle, filter1);
	void *filter3 = ffmpeg_filter_alloc_filter(filterhandle, "buffersink", "buffersink", NULL);
	ffmpeg_filter_set_video_sink_filter(filterhandle, filter3);
	ffmpeg_filter_link_filter(filter1, filter3);

	ffmpeg_filter_open(filterhandle);
	ffmpeg_filter_set_video(filterhandle, 960, 540, 1);

	std::fstream fs;
	fs.open("1.yuv", std::ios::binary|std::ios::out);

	while( 1 )
	{
		const char *frame = NULL;
		int length = 0;
		video_generator_get_yuv420p_frame(handle, &frame, &length);

		ffmpeg_filter_set_video_data(filterhandle, frame, length);

		if( ffmpeg_filter_get_video_data(filterhandle, &frame, &length) >= 0 )
		{
			if( length > 0 )
			{
				printf("frame length = %d \n", length);
				fs.write(frame,length);				
			}			
		}

		usleep(100 * 1000);
	}

	return 0;
}