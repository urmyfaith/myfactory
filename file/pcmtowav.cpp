#include <stdio.h>
#include <unistd.h>
#include <fstream>

#include "libwav/wavmux.h"
#include "libpcm/pcmdemux.h"

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

//	void* wavhandle = wavfile_open(argv[2], WAV_AUDIOFORMAT_PCM, config.samplerate, config.bitcount, config.channelcount);
//	void* wavhandle = wavfile_open(argv[2], WAV_AUDIOFORMAT_ALAW, config.samplerate, config.bitcount, config.channelcount);
	void* wavhandle = wavfile_open(argv[2], WAV_AUDIOFORMAT_ULAW, config.samplerate, config.bitcount, config.channelcount);

	const char *pcmframe = NULL;
	int framelength = -1;
	while( 1 )
	{
		int ret = PCMDemux_GetFrame(pcmhandle, &pcmframe, &framelength);
		if( ret < 0 )
		{
			printf("ReadOneNaluFromBuf error\n");
			break;
		}
        
		ret = wavfile_write(wavhandle, pcmframe, framelength);
		if( ret < 0 )
		{
			printf("ReadOneNaluFromBuf error\n");
			break;
		}
	}

	wavfile_close(wavhandle);
    PCMDemux_CLose(pcmhandle);

    return 0;
}
