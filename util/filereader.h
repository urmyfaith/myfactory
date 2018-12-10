#ifndef __FILEREADER_H__
#define __FILEREADER_H__

void *filereader_alloc(char* filepath, int isbinary);

int filereader_write(void *handle, char* data, int length);

int filereader_free(void *handle);

#endif