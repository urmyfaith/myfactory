#ifndef __URL_MUX_H__
#define __URL_MUX_H__
             
#include <stdint.h>

int url_mux_alloc(void** inst);

int url_mux_set_request_line(void* inst, char* method, char* url, char* version);
int url_mux_set_status_line(void* inst, char* version, char* errorCode, char* errorDescription);

int url_mux_set_header(void* inst, char* headername, char* headervalue);

int url_mux_set_body(void* inst, char* bodyvalue, int bodylength);

int url_mux_get_buffer(void* inst, char** buffer, int *length);
int url_mux_flush_buffer(void* inst);

int url_mux_free(void* inst);

#endif