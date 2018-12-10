// FFTest.cpp : Defines the entry point for the application.
//

#include <string>
#include <queue>

#include "player.h"

int main(int argc, char* argv[])
{
	if( argc < 2 )
		return -1;
	
	void* handle = player_open();
	player_play(handle, argv[1]);
	player_close(handle);

	return 0;

//	return player_playurl(argv[1], 0);
}