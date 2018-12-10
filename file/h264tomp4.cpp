#include <stdio.h>
#include <unistd.h>
#include <fstream>

#include "libh26x/h264demux.h"
#include "libmp4/mp4mux.h"

int test264tomp4(char* h264path, char* mp4path)
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
	printf("H264Demux_GetConfig:width %d height %d framerate %d timescale %d\n",
			config.width, config.height, config.framerate, config.timescale);

	MP4Encoder mp4Encoder;  
    int ret = mp4Encoder.CreateMP4File(mp4path);
	if( ret < 0 )
	{
		printf("CreateMP4File error\n");
		return -1;
	}
	ret = mp4Encoder.Write264DecoderConfiguration(config.sps, config.spslen, 
		config.pps, config.ppslen, config.width, config.height, 
		config.timescale, config.framerate);
	if( ret < 0 )
	{
		printf("Write264DecoderConfiguration error\n");
		return -1;
	}

	const char *h264frame = NULL;
	int framelength = -1;
	while( 1 )
	{
		ret = H264Demux_GetFrame(h264handle, &h264frame, &framelength);
		if( ret < 0 )
		{
			printf("ReadOneNaluFromBuf error\n");
			break;
		}
        
        ret = mp4Encoder.WriteH264Data(h264frame+config.headerlen, framelength-config.headerlen);     
        if( ret < 0 )
            break;
        
//		usleep(50 * 1000);
	}

    mp4Encoder.CloseMP4File();
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
    
	return test264tomp4(argv[1], argv[2]);
}
