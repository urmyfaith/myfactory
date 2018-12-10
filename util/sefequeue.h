#ifndef __SAFEQUEUE_H__
#define __SAFEQUEUE_H__

#include <queue>
#include <mutex>

template <class T>
class safequeue
{
public:
	safequeue();
	virtual ~safequeue();

	int pushback(T &value);
	int getfront(T &value);
	int popfront();
	
private:
	std::queue<T> dataqueue;
	std::mutex datamutex;
};

#endif