#include <stdio.h>

#include "confmanager.h"
#include "../../../util/thread.h"
#include "../../../transport/udp/udpselectserver.h"

int confmanager_proc(void* usrdata)
{
	confmanager* obj = (confmanager*)usrdata;

   	obj->runloop();
}

////////////////////////////////////////////////////
confmanager::confmanager()
{
    int ret = thread_alloc(&threadhandle, confmanager_proc, this);
}

confmanager::~confmanager()
{
	thread_free(threadhandle);
}

int confmanager::setpacket(int localfd, char* data, int length, char* remoteip, int remoteport)
{
	confmutex.lock();
	confmanagermap::iterator iter = confmap.find(remoteport);
	if( iter ==confmap.end() )
	{
		remoteinfo_tag_t info;
		info.localfd = localfd;
		info.remoteip = remoteip;
		info.remoteport= remoteport;

		info.buffer.clear();
		info.buffer.append(data, length);
		info.rtppackets.push(info.buffer);
		confmap[remoteport] = info;
	}
	else
	{
		iter->second.buffer.clear();
		iter->second.buffer.append(data, length);
		iter->second.rtppackets.push(iter->second.buffer);
	}

	confmutex.unlock();

	return 0;
}

int confmanager::runloop()
{
    while( thread_testcancel(threadhandle) )
    {
		usleep(1000 * 1);

    	confmutex.lock();
    	//只有一个人时不转发
    	if( confmap.empty() || confmap.size() == 1 )
    	{
	    	confmutex.unlock();
    		usleep(1000 * 1000);
    		continue;
    	}

    	for(confmanagermap::iterator iter = confmap.begin();
    		iter != confmap.end();
    		++iter)
    	{
    		if( iter->second.rtppackets.empty() )
    			continue;

    		std::string &rtppacket = iter->second.rtppackets.front();
	    	for(confmanagermap::iterator subiter = confmap.begin();
	    		subiter != confmap.end();
	    		++subiter)
	    	{
	    		//不能自己发给自己
	    		if( subiter->first == iter->first )
	    			continue;

				int ret = udp_select_server_write(subiter->second.localfd, 
					rtppacket.data(), rtppacket.size(), 
					(char*)subiter->second.remoteip.c_str(), 
					subiter->second.remoteport);
	    	}
	    	iter->second.rtppackets.pop();
    	}

    	confmutex.unlock();
    }
}