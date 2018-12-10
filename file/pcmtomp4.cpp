#include <stdio.h>
#include <unistd.h>
#include <fstream>

#include "libwmp4/mp4mux.h"
#include "libpcmplugin/pcmdemux.h"

int main(int argc, char* argv[])
{
	if( argc < 3 )
	{
		printf("parameter error\n");
		return -1;
	}    
    
	void* pcmhandle = PCMDemux_Init(argv[1], 16, 2, 44100, 0);
	if( !pcmhandle )
	{
		printf("PCMDemux_Init error\n");
		return -1;
	}
	PCMConfiguration_t config;
	if( PCMDemux_GetConfig(pcmhandle, &config) < 0 )
	{
		printf("PCMDemux_GetConfig error\n");
		return -1;
	}
	printf("PCMDemux_GetConfig:samplerate %d profilelevel %d %d %d\n",
			config.samplerate, config.profilelevel, config.samplerateindex,
			config.channelcount);

	MP4Encoder mp4Encoder;  
    int ret = mp4Encoder.CreateMP4File(argv[2]);
	if( ret < 0 )
	{
		printf("CreateMP4File error\n");
		return -1;
	}
	ret = mp4Encoder.WritePCMDecoderConfiguration(config.samplerate, config.profilelevel);
	if( ret < 0 )
	{
		printf("CreateMP4File error\n");
		return -1;
	}

	const char *pcmframe = NULL;
	int framelength = -1;
	while( 1 )
	{
		ret = PCMDemux_GetFrame(pcmhandle, &pcmframe, &framelength);
		if( ret < 0 )
		{
			printf("ReadOneNaluFromBuf error\n");
			break;
		}
        
        ret = mp4Encoder.WriteAACData(pcmframe, framelength);     
        if( ret < 0 )
            break;
	}

    mp4Encoder.CloseMP4File();
    PCMDemux_CLose(pcmhandle);

    return 0;
}
