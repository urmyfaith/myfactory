#ifndef __DEMUXER_H__
#define __DEMUXER_H__

void* demuxer_alloc(const char* sourcename);

int demuxer_getframe(void* handle, const char** frame, int *length);

int demuxer_free(void *handle);

#endif