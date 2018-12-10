#ifndef __TSMUX2_H__
#define __TSMUX2_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <fstream>

//////////////////////////////////////////////////////////////////
void *es2ts_alloc(const char* filename);

//h264
int es2ts_addvideostream(void *handle);

//aac
int es2ts_addaudiostream(void *handle);

int es2ts_write_frame(void *handle, const char* framedata, int nFrameLen, uint64_t timestamp, int isvideo);

int es2ts_destroy(void *handle);

#endif