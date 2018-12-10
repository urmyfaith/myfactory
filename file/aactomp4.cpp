#include <stdio.h>
#include <unistd.h>
#include <fstream>

#include "libaac/aacdemux.h"
#include "libmp4/mp4mux.h"

int testaactomp4(char* aacpath, char* mp4path)
{
	void *aachandle = AACDemux_Init(aacpath, 0);
	if( !aachandle )
	{
		printf("AACDemux_Init error\n");
		return -1;
	}
	AACConfiguration_t config;
	if( AACDemux_GetConfig(aachandle, &config) < 0 )
	{
		printf("AACDemux_GetConfig error\n");
		return -1;
	}
	printf("AACDemux_GetConfig:samplerate %d profilelevel %d \n",
			config.samplerate, config.profilelevel);

	MP4Encoder mp4Encoder;  
    int ret = mp4Encoder.CreateMP4File(mp4path);
	if( ret < 0 )
	{
		printf("CreateMP4File error\n");
		return -1;
	}
	mp4Encoder.WriteAACDecoderConfiguration(config.samplerate, config.profilelevel);

	const char *aacframe = NULL;
	int framelength = -1;
	while( 1 )
	{
		ret = AACDemux_GetFrame(aachandle, &aacframe, &framelength);
		if( ret < 0 )
		{
			printf("ReadOneNaluFromBuf error\n");
			break;
		}
        
        ret = mp4Encoder.WriteAACData(aacframe+config.headerlen, framelength-config.headerlen);     
        if( ret < 0 )
            break;
        
//		usleep(50 * 1000);
	}

    mp4Encoder.CloseMP4File();
    AACDemux_CLose(aachandle);

    return 0;
}

int main(int argc, char* argv[])
{
	if( argc < 3 )
	{
		printf("parameter error\n");
		return -1;
	}    
    
	testaactomp4(argv[1], argv[2]);

	return 0;
}
