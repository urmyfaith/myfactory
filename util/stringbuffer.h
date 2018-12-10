#ifndef __STRINGBUFFER_H__
#define __STRINGBUFFER_H__

#include <inttypes.h>

void* stringbuffer_alloc(int initialsize);
uint32_t stringbuffer_getsize(void *handle);
char* stringbuffer_getbuffer(void *handle);
int stringbuffer_append(void *handle, const char* data, int length);
int stringbuffer_erasefromhead(void *handle, int length);
int stringbuffer_free(void *handle);

#endif