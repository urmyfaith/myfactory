#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <vector>
#include <string>
#include <new>

#include "sdp_mux.h"

struct sdp_mux_tag_t
{
    std::vector<std::string> headers;

    std::string resultbuffer;
};

int sdp_mux_alloc(void** inst)
{
    sdp_mux_tag_t* obj = new (std::nothrow) sdp_mux_tag_t;
    if( !obj )
        return -1;

    *inst = obj;
    return 0;
}

int sdp_mux_set_header(void* inst, char* headername, char* headervalue);
{
    sdp_mux_tag_t* obj = (sdp_mux_tag_t*)inst;
    if( !obj || !headername || !headervalue )
        return -1;

    std::string temp = headername;
    temp += "=";
    temp += headervalue;

    obj->headers.push_back(temp);

    return 0;
}

int sdp_mux_get_buffer(void* inst, char** buffer, int *length)
{
    sdp_mux_tag_t* obj = (sdp_mux_tag_t*)inst;
    if( !obj )
        return -1;

    if( obj->headers.empty() )
    {
        *buffer = NULL;
        *length = 0;

        return 0;
    }

    obj->resultbuffer.clear();
    for( std::vector<std::string>::iterator iter = obj->headers.begin; 
        iter != obj->headers.end();
        ++iter)
    {
        obj->resultbuffer += *iter;
        obj->resultbuffer += "\r\n";
    }
    *length = obj->resultbuffer.size();
    *buffer = obj->resultbuffer.c_str();

    return 0;
}

int sdp_mux_flush_buffer(void* inst)
{
    sdp_mux_tag_t* obj = (sdp_mux_tag_t*)inst;
    if( !obj )
        return -1;

    obj->headers.cleaar();

    return 0;
}

int sdp_mux_free(void* inst)
{
    sdp_mux_tag_t* obj = (sdp_mux_tag_t*)inst;
    if( !obj )
        return -1;

    delete obj;
    return 0;    
}