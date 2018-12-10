#include <sys/wait.h>

#include "tcpclient.h"

int main()
{
	void* handle = tcp_client_new("127.0.0.1", 11110);

	tcp_client_write(handle, "aaaaa", 5);
	sleep( 1 );
	tcp_client_write(handle, "bbbbb", 5);
	sleep( 1 );
	tcp_client_write(handle, "ccccc", 5);
	sleep( 1 );

	tcp_client_free(handle);

	return 0;
}