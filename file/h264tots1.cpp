#include <stdio.h>
#include <unistd.h>
#include <fstream>

#include "libh26x/h264demux.h"
#include "libmpeg/MuxTs.h"

static int g_frame_count = 0;

int test264tompegts(char* h264path, char* tspath)
{
	void* h264handle = H264Demux_Init(h264path, 0);
	if( !h264handle )
	{
		printf("H264Framer_Init error\n");
		return -1;
	}
	H264Configuration_t config;
	if( H264Demux_GetConfig(h264handle, &config) < 0 )
	{
		printf("H264Demux_GetConfig error\n");
		return -1;
	}
	printf("H264Demux_GetConfig:width %d height %d framerate %d timescale %d %d %d \n",
			config.width, config.height, config.framerate, config.timescale,
			config.spslen, config.ppslen);

	MuxTs m;
	m.CreateFile(tspath);
	m.AddNewProgram(0x1000, 1);
	m.AddNewStream(0x101, MUXTS_CODEC_H264);

	const char *h264frame = NULL;
	int framelength = -1;
	while( 1 )
	{
		int ret = H264Demux_GetFrame(h264handle, &h264frame, &framelength);
		if( ret < 0 )
		{
			printf("ReadOneNaluFromBuf error\n");
			break;
		}

		printf("framelength %d \n",framelength);
		m.WriteFrame((unsigned char*)h264frame, framelength, 3600L * g_frame_count, MUXTS_CODEC_H264);

		g_frame_count++;

		if( framelength > 100 )
			break;
	}

	m.CloseFile();
    H264Demux_CLose(h264handle);

    return 0;
}

int main(int argc, char* argv[])
{
	if( argc < 3 )
	{
		printf("parameter error\n");
		return -1;
	}    
    
	test264tompegts(argv[1], argv[2]);

	return 0;
}
