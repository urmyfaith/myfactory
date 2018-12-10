#ifndef __FILEWRITTER_H__
#define __FILEWRITTER_H__

void *filewritter_alloc(char* filepath, int isbinary);

int filewritter_write(void *handle, char* data, int length);

int filewritter_free(void *handle);

#endif