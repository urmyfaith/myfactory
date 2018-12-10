#ifndef __CONFMANAGER_H__
#define __CONFMANAGER_H__

#include <queue>
#include <mutex>
#include <map>
#include <string>

struct remoteinfo_tag_t
{
	int localfd;
	std::string remoteip;
	int remoteport;
	std::string buffer;
	std::queue<std::string> rtppackets;
};

typedef std::map<int, remoteinfo_tag_t> confmanagermap;

class confmanager
{
public:
	confmanager();
	~confmanager();
	int setpacket(int localfd, char* data, int length, char* remoteip, int remoteport);

	int remove_endpoint(int key);

	int runloop();

private:
	confmanagermap confmap;
	std::mutex confmutex;
	void* threadhandle;
};

#endif