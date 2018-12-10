#ifndef __UDP_SELECT_SERVER__
#define __UDP_SELECT_SERVER__

#include <unistd.h>
#include <stdio.h>

typedef int (*on_recv_callback)(void* handle, char* data, int length, 
			int localfd, char* remoteip, int remoteport, void* userdata);

void* udp_select_server_new(const char* localip, int localport, on_recv_callback recv_callback, void* userdata);

int udp_select_server_write(int localfd, const char* data, int length, char* remoteip, int remoteport);

int udp_select_server_free(void* handle);

#endif