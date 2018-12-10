#include <sys/wait.h>
#include <stdio.h>
#include <string.h>

#include "udpserver.h"

int sockfd = -1;
int myon_connect_callback(void* handle, int sockfd, void* userdata)
{
	printf("myon_connect_callback %d \n", sockfd);
	sockfd = sockfd;
	
	return 0;
}

int myon_close_callback(void* handle, int sockfd, void* userdata)
{
	printf("myon_close_callback %d \n", sockfd);

	return 0;
}

int main()
{
	void* handle = udp_server_new("127.0.0.1", 11110, myon_connect_callback, myon_close_callback, NULL);

//int tcp_server_write(void* handle, int sockfd, const char* data, int length);

	char buffer[1024] = {0};	
	while( 1 )
	{
		memset(buffer, 0, sizeof(buffer));
		int ret = udp_server_read(handle, sockfd, buffer, sizeof(buffer));

		if( ret > 0 )
			printf("%s %d\n", buffer, ret);

		sleep( 1 );
	}

	udp_server_free(handle);

	return 0;
}