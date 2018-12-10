#ifndef __URL_DEMUX_H__
#define __URL_DEMUX_H__

#include <stdint.h>

struct url_demux_header
{
	char* headername;
	char* headervalue;
};

int url_demux_alloc(void** inst);

int url_demux_requst(void* inst, char* request, int length, char** method, char** url, char** version, 
			url_demux_header **headers, int *headercount, char** body, int *bodylength);

int url_demux_response(void* inst, char* response, int length, char** version, char** errorCode, char** errorDescription, 
			url_demux_header **headers, int *headercount, char** body, int *bodylength);

int url_demux_free(void* inst);

#endif