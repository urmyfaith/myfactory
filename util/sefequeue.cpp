
#include "safequeue.h"

safequeue::safequeue()
{

}

safequeue::~safequeue()
{
	datamutex.lock();
	dataqueue.clear();
	datamutex.unlock();
}

int safequeue::pushback(T &value)
{
	datamutex.lock();
	dataqueue.push_back(value);
	datamutex.unlock();

	return 0;
}

int safequeue::getfront(T &value)
{
	datamutex.lock();
	if( dataqueue.size() <= 0 )
	{
		datamutex.unlock();
		return -1;
	}
	value = dataqueue.front();
	datamutex.unlock();

	return 0;

}

int safequeue::popfront()
{
	datamutex.lock();
	if( dataqueue.size() <= 0 )
	{
		datamutex.unlock();
		return -1;
	}
	dataqueue.pop_front();
	datamutex.unlock();

	return 0;

}