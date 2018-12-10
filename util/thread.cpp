#include <thread>

#include "thread.h"

struct thread_tag_t
{
	std::thread threadloop;
	volatile int isrunning;
	thread_proc proc;
	void* usrdata;
};

int thread_alloc(void** handle, thread_proc proc, void* usrdata)
{
	if( !proc )
		return -1;

	thread_tag_t *inst = new thread_tag_t;
	if( !inst )
		return -1;

	inst->isrunning = 1;
	inst->proc = proc;
	inst->usrdata = usrdata;
	inst->threadloop = std::thread(proc, usrdata);

	*handle = inst;

	return 0;
}

int thread_testcancel(void *handle)
{
	thread_tag_t *inst = (thread_tag_t*)handle;
	return inst->isrunning;	
}

int thread_free(void *handle)
{
	thread_tag_t *inst = (thread_tag_t*)handle;

	inst->isrunning = 0;
	inst->threadloop.join();

	delete inst;

	return 0;
}
