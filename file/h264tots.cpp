#include <stdio.h>
#include <unistd.h>
#include <fstream>

#include "libh26x/h264demux.h"
#include "libmpeg/tsmux.h"

#define BUF_SIZE (1<<20)

static int g_is_ps = 0;
static FILE *g_out_fp;
static TsProgramInfo g_prog_info;
static int g_frame_count = 0;
static uint8_t g_outbuf[BUF_SIZE] = {0};
static TDemux g_demux;

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

	g_out_fp = fopen(tspath, "wb");
	memset(&g_prog_info, 0, sizeof(g_prog_info));
	g_prog_info.program_num = 1;
	g_prog_info.prog[0].stream_num = 1;
	g_prog_info.prog[0].stream[0].type = STREAM_TYPE_VIDEO_H264;

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

		TEsFrame es = {0};
		es.program_number = 0;
		es.stream_number = 0;
		es.frame = (uint8_t*)h264frame;
		es.length = framelength;
		es.is_key = framelength < 100? 1:0;					// 这里简单处理，认为信息帧（非数据帧）为关键帧。
		es.pts = 3600L * g_frame_count;		// 示例中按帧率为25fps累计时间戳。正式使用应根据帧实际的时间戳填写。
		es.ps_pes_length = 8000;

		printf("framelength %d \n",framelength);
		int outlen = lts_ts_stream(&es, g_outbuf, BUF_SIZE, &g_prog_info);
		if (outlen > 0)
		{
			fwrite(g_outbuf, 1, outlen, g_out_fp);
		}

		g_frame_count++;
		if( framelength > 100 )
			break;
	}

	fclose(g_out_fp);
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
