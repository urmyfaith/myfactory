#include <stdio.h>
#include <unistd.h>
#include <fstream>

#include "libh26x/h264demux.h"
#include "libflv/flvmux.h"

int test264toflv(char* h264path, char* flvpath)
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

	Cnvt::CConverter conv;
	int ret = conv.Open(flvpath, 0, 1);
	if( ret < 0 )
	{
		printf("Openflv error\n");
		return -1;
	}
	conv.WriteH264Header((unsigned char*)config.sps, (int)config.spslen, (unsigned char*)config.pps, (int)config.ppslen );

	const char *h264frame = NULL;
	int framelength = -1;
	unsigned int timestamp = 0;

	while( 1 )
	{
		ret = H264Demux_GetFrame(h264handle, &h264frame, &framelength);
		if( ret < 0 )
		{
			printf("ReadOneNaluFromBuf error\n");
			break;
		}

		ret = conv.WriteH264Frame(h264frame+config.headerlen, framelength-config.headerlen, timestamp);
        if( ret < 0 )
            break;

		timestamp += 50;        
	}

    conv.Close();
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
    
	return test264toflv(argv[1], argv[2]);
}
