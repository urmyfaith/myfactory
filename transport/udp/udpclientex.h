#ifndef __UDP_CLIENTEX__
#define __UDP_CLIENTEX__

#include <unistd.h>
#include <stdio.h>

void* udp_clientex_new(int localport, int localfd);

int udp_clientex_write(void* handle, const char * sendBuff, int length, 
        const char* remoteip, int remoteport);

int udp_clientex_selectread(void* handle);

int udp_clientex_read(void* handle, char* buffer, int length,
	const char** remoteip, int *remoteport);

int udp_clientex_free(void* handle);

#endif