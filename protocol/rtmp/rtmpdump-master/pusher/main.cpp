#include <stdio.h>     
#include "rtmpstream.h"     
    
CRTMPStream rtmpSender;    
int main(int argc,char* argv[])    
{
	if( argc < 2 )
	{
		printf("parameter: h264filepath \n");
		return -1;
	}

    if( !rtmpSender.Connect("rtmp://127.0.0.1:1935/live/test") )
    {
    	printf("Connect error \n");
    	return -1;
    }
    
    rtmpSender.SendH264File(argv[1]);    
    
    rtmpSender.Close();    

    return 0;
}
