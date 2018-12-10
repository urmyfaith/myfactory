#include <stdio.h>  
#include <sstream>
#include <unistd.h>

#include "aacdemux.h"
#include "h264demux.h"
#include "tsmux2.h"

int main()
{
	void* handle = AACDemux_Init("song.aac", 1);
	void* h264handle = H264Demux_Init("ts.h264", 1);
	if( !h264handle )
	{
		printf("H264Framer_Init error\n");
		return -1;
	}

	void* tshandle = es2ts_alloc("test.ts");
	es2ts_addvideostream(tshandle);
	es2ts_addaudiostream(tshandle);

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
		es2ts_write_frame(tshandle, frame, length, pts, 0);

		ret = H264Demux_GetFrame(h264handle, &h264frame, &framelength);
		if( ret < 0 )
		{
			printf("ReadOneNaluFromBuf error\n");
			break;
		}
		int frametype = h264frame[4]&0x1F;
		printf("frametype %d\n", frametype);
		es2ts_write_frame(tshandle, h264frame, framelength, pts, 1);

		pts += 3600;
		
		usleep(10 * 1000);
	}

	es2ts_destroy(tshandle);
	return 0;
}