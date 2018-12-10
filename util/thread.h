#ifndef __THREAD_H__
#define __THREAD_H__

typedef int (*thread_proc)(void *usrdata);

int thread_alloc(void** handle, thread_proc proc, void* usrdata);

int thread_testcancel(void *handle);

int thread_free(void *handle);

#endif