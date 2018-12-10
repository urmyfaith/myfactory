#include <stdio.h>  
#include <sstream>
#include <unistd.h>

#include "aacdemux.h"
#include "h264demux.h"

#include "tsfile2.h"
#include "ffmpeg_mux.h"
#include "tsfile.h"

int testts1()
{
	void* handle = AACDemux_Init("song.aac", 1);
	void* h264handle = H264Demux_Init("ts.h264", 1);
	if( !h264handle )
	{
		printf("H264Framer_Init error\n");
		return -1;
	}

	tsfilewriter2 tsfile;
	tsfile.open_file("aac0.ts");

	const char *frame = NULL;
	int length = 0;
	const char *h264frame = NULL;
	int framelength = -1;

	int filecounter = 0;
	int keyframecounter = 0;
	std::stringstream ss;

	uint64_t pts = 0;
	int ret = 0;
	while( 1 )
	{
		ret = AACDemux_GetFrame(handle, &frame, &length);
		if( ret < 0 )
			break;
		tsfile.write_aac_pes(frame, length, pts);

		ret = H264Demux_GetFrame(h264handle, &h264frame, &framelength);
		if( ret < 0 )
		{
			printf("ReadOneNaluFromBuf error\n");
			break;
		}
		int frametype = h264frame[4]&0x1F;
		printf("frametype %d\n", frametype);
		if( frametype == 7 )
		{
			keyframecounter++;
			if( keyframecounter > 5 )
			{
				keyframecounter = 0;
				pts = 0;
				
				printf("new file \n");
				tsfile.close_file();
				filecounter++;
				ss<<"aac"<<filecounter<<".ts";
				tsfile.open_file(ss.str().c_str());
				ss.str("");
			}
		}
		tsfile.write_h264_pes(h264frame, framelength, pts);

		pts += 3600;
		
		usleep(100 * 1000);
	}

	return 0;
}

int testts2()
{
	void* handle = AACDemux_Init("song.aac", 1);
	void* h264handle = H264Demux_Init("ts.h264", 0);

	void* muxhandle = ffmpegmux_alloc("mux0.ts");
	ffmpegmux_addvideostream(muxhandle, 352, 288);
	ffmpegmux_addaudiostream(muxhandle, 44100, 2);
	ffmpegmux_open(muxhandle);

	printf("ffmpegmux_open \n");
	
	const char *frame = NULL;
	int length = 0;
	const char *h264frame = NULL;
	int framelength = -1;

	int filecounter = 0;
	int keyframecounter = 0;
	std::stringstream ss;

	uint64_t pts = 0;
	uint64_t audiopts = 0;
	while( 1 )
	{
		int ret = AACDemux_GetFrame(handle, &frame, &length);
		if( ret < 0 )
			continue;
		printf("ffmpegmux_write_audio frame \n");
		ffmpegmux_write_audio_frame(muxhandle, frame, length, audiopts);
		audiopts+=3600;

		ret = H264Demux_GetFrame(h264handle, &h264frame, &framelength);
		if( ret < 0 )
		{
			printf("ReadOneNaluFromBuf error\n");
			break;
		}
		int frametype = h264frame[4]&0x1F;
		printf("frametype %d\n", frametype);
		if( frametype == 7 )
		{
			keyframecounter++;
			if( keyframecounter > 5 )
			{
				keyframecounter = 0;
				pts = 0;
				audiopts = 0;

				printf("new file \n");
				ffmpegmux_destroy(muxhandle);
				filecounter++;
				ss<<"mux"<<filecounter<<".ts";
				muxhandle = ffmpegmux_alloc(ss.str().c_str());
				ffmpegmux_addvideostream(muxhandle, 352, 288);
				ffmpegmux_addaudiostream(muxhandle, 44100, 2);
				ffmpegmux_open(muxhandle);
				ss.str("");
			}
		}

		printf("ffmpegmux_write_video frame \n");
		ffmpegmux_write_video_frame(muxhandle, h264frame, framelength, pts);

		pts += 3600;
		usleep(100 * 1000);
	}

	return 0;
}

int testts3()
{
	void* handle = AACDemux_Init("song.aac", 1);
	void* h264handle = H264Demux_Init("ts.h264", 0);

	tsfilewriter tswritter;
	tswritter.open_file("ts0.ts");
	tswritter.write_pat();
	tswritter.write_pmt();

	printf("ffmpegmux_open \n");
	
	const char *frame = NULL;
	int length = 0;
	const char *h264frame = NULL;
	int framelength = -1;

	int filecounter = 0;
	int keyframecounter = 0;
	std::stringstream ss;

	uint64_t pts = 0;
	uint64_t audiopts = 0;
	while( 1 )
	{
		int ret = AACDemux_GetFrame(handle, &frame, &length);
		if( ret < 0 )
			continue;
		printf("ffmpegmux_write_audio frame \n");
		tswritter.write_aac_pes(frame, length, audiopts);
		audiopts+=3600;

		ret = H264Demux_GetFrame(h264handle, &h264frame, &framelength);
		if( ret < 0 )
		{
			printf("ReadOneNaluFromBuf error\n");
			break;
		}
		int frametype = h264frame[4]&0x1F;
		printf("frametype %d\n", frametype);
		if( frametype == 7 )
		{
			keyframecounter++;
			if( keyframecounter > 5 )
			{
				keyframecounter = 0;
				pts = 0;
				audiopts = 0;

				printf("new file \n");
				filecounter++;
				ss<<"ts"<<filecounter<<".ts";

				tswritter.close_file();				
				tswritter.open_file(ss.str().c_str());
				tswritter.write_pat();
				tswritter.write_pmt();
				ss.str("");
			}
		}

		printf("ffmpegmux_write_video frame \n");
		tswritter.write_h264_pes(h264frame, framelength, pts);

		pts += 3600;
		usleep(100 * 1000);
	}

	return 0;
}
