#ifndef __MULTIDEST_MUX_H__
#define __MULTIDEST_MUX_H__

#include <unistd.h>
#include <stdio.h>

void* multidest_mux_new();

int multidest_mux_add_ipport(void* handle, const char * ipv4,uint16_t port);

int multidest_mux_get_buffer_for_net(void* handle, const char** buffer, int *length);

int multidest_mux_free(void* handle);

#endif