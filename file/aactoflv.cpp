#include <stdio.h>
#include <unistd.h>
#include <fstream>

#include "libaac/aacdemux.h"
#include "libflv/flvmux.h"

int testaactoflv(char* aacpath, char* flvpath)
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
	printf("AACDemux_GetConfig:samplerate %d profilelevel %d %d %d\n",
			config.samplerate, config.profilelevel, config.samplerateindex,
			config.channelcount);

	Cnvt::CConverter conv;
	int ret = conv.Open(flvpath, 1, 0);
	if( ret < 0 )
	{
		printf("Openflv error\n");
		return -1;
	}
	conv.WriteAACHeader(config.profilelevel, config.samplerateindex, 
		config.channelcount);

	const char *aacframe = NULL;
	int framelength = -1;
	unsigned int timestamp = 0;

	while( 1 )
	{
		ret = AACDemux_GetFrame(aachandle, &aacframe, &framelength);
		if( ret < 0 )
		{
			printf("ReadOneNaluFromBuf error\n");
			break;
		}
        
		ret = conv.WriteAACFrame(aacframe+config.headerlen, framelength-config.headerlen, timestamp);
        if( ret < 0 )
            break;

		timestamp += 50;        
	}

    conv.Close();
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
    
	testaactoflv(argv[1], argv[2]);

	return 0;
}
