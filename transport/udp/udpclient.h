#ifndef __UDP_CLIENT__
#define __UDP_CLIENT__

#include <unistd.h>
#include <stdio.h>

void* udp_client_new(const char * server,int serverPort, int localport, int localfd);

int udp_client_write(void* handle, const char * sendBuff, int length);

int udp_client_read(void* handle, char* buffer, int length);

int udp_client_free(void* handle);

#endif