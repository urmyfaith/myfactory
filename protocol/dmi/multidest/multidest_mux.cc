#include   <sys/stat.h>     
#include   <sys/types.h>     
#include   <sys/socket.h>     
#include   <stdio.h>     
#include   <malloc.h>     
#include   <netdb.h>     
#include   <fcntl.h>  
#include   <unistd.h>  
#include   <netinet/in.h>  
#include   <arpa/inet.h>  
#include  <string.h>
#include <string>

#include "multidest_mux.h"

struct multidestmux_t {
    std::string databuffer;
};

void* multidest_mux_new()
{
    multidestmux_t *inst = new multidestmux_t;

    return inst;
}

int multidest_mux_add_ipport(void* handle, const char * ipv4,uint16_t port)
{
    multidestmux_t *inst = (multidestmux_t*)handle;

    uint32_t ip = inet_addr(ipv4);
    inst->databuffer.append((const char*)&ip, sizeof(ip));

    uint16_t uport = htons(port);
    inst->databuffer.append((const char*)&uport, sizeof(uport));

    return 0;
}

int multidest_mux_get_buffer_for_net(void* handle, const char** buffer, int *length)
{
    multidestmux_t *inst = (multidestmux_t*)handle;

    *buffer = inst->databuffer.data();
    *length = inst->databuffer.size();

    return 0;
}

int multidest_mux_free(void* handle)
{
    if( !handle )
        return -1;

    multidestmux_t *inst = (multidestmux_t*)handle;

    delete inst;

    return 0;
}
