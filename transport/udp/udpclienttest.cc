#include <sys/wait.h>

#include "udpclient.h"

int main()
{
	void* handle = udp_client_new("127.0.0.1", 11110);
	sleep( 1 );

	int ret = udp_client_write(handle, "connect", 7);
	if( ret < 0 )
	{
		printf("udp_client_write error \n");
		return -1;
	}
	sleep( 1 );

	udp_client_write(handle, "aaaaa", 5);
	if( ret < 0 )
	{
		printf("udp_client_write error \n");
		return -1;
	}

	udp_client_write(handle, "bbbbb", 5);
	if( ret < 0 )
	{
		printf("udp_client_write error \n");
		return -1;
	}

	while( 1 )
	{
		sleep( 1 );		
	}

	udp_client_free(handle);

	return 0;
}