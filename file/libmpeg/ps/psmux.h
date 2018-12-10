#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

void *psmux_alloc(const char* filename);

//h264
int psmux_addvideostream(void *handle);

//aac
int psmux_addaudiostream(void *handle);

int psmux_writeframe(void* handle, const char *pData, int nFrameLen, uint64_t timestamp, int isvideo);

int psmux_free(void *handle);