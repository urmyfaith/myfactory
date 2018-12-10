#include <stdio.h>

#include "../../../transport/udp/udpselectserver.h"

#include "confmanager.h"

confmanager confman;

int on_udprecv_callback(void* handle, char* data, int length, int localfd, 
			char* remoteip, int remoteport, void* userdata)
{

	printf("on_udprecv_callback length=%d, remoteport=%d\n", length, remoteport);

	confman.setpacket(localfd, data, length, remoteip, remoteport);
	//int ret = udp_select_server_write(localfd, (const char*)data, length, remoteip, remoteport);

	return 0;
}

int udpserv()
{
    const char* localip = "192.168.0.100";//"127.0.0.1";
    void* udpserverhandle = udp_select_server_new(localip, 11011, on_udprecv_callback, NULL);
	if( !udpserverhandle )
	{
		printf("udp_select_server_new error\n");
		return -1;
	}
	
	while( 1 )
	{
        usleep(1000 * 2000);
	}
	return 0;
}

int main()
{
	udpserv();

	return 0;
}