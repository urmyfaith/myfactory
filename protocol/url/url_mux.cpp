#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <map>
#include <string>
#include <new>

#include "url_mux.h"

struct url_mux_tag_t
{
    std::map<std::string, std::string> headers;

    std::string body;

    std::string headline;

    std::string resultbuffer;
};

int url_mux_alloc(void** inst)
{
    url_mux_tag_t* obj = new (std::nothrow) url_mux_tag_t;
    if( !obj )
        return -1;

    *inst = obj;
    return 0;
}

int url_mux_set_request_line(void* inst, char* method, char* url, char* version)
{
    url_mux_tag_t* obj = (url_mux_tag_t*)inst;
    if( !obj )
        return -1;

    obj->headline = method;
    obj->headline += " ";
    obj->headline += url;
    obj->headline += " ";
    obj->headline += version;

    return 0;
}

int url_mux_set_status_line(void* inst, char* version, char* errorCode, char* errorDescription)
{
    url_mux_tag_t* obj = (url_mux_tag_t*)inst;
    if( !obj )
        return -1;

    obj->headline = version;
    obj->headline += " ";
    obj->headline += errorCode;
    obj->headline += " ";
    obj->headline += errorDescription;

    return 0;    
}

int url_mux_set_header(void* inst, char* headername, char* headervalue)
{
    url_mux_tag_t* obj = (url_mux_tag_t*)inst;
    if( !obj )
        return -1;

    obj->headers[headername] = headervalue;

    return 0;    
}

int url_mux_set_body(void* inst, char* bodyvalue, int bodylength)
{
    url_mux_tag_t* obj = (url_mux_tag_t*)inst;
    if( !obj )
        return -1;

    obj->body.clear();
    boj->body.append(bodyvalue, bodylength);

    return 0;    
}

int url_mux_get_buffer(void* inst, char** buffer, int *length)
{
    url_mux_tag_t* obj = (url_mux_tag_t*)inst;
    if( !obj )
        return -1;

    obj->resultbuffer = obj->headline;
    obj->resultbuffer += "\r\n";

    if( !obj->headers.empty() )
    {
        for( std::map<std::string, std::string>::iterator iter = obj->headers.begin; 
            iter != obj->headers.end();
            ++iter)
        {
            obj->resultbuffer += iter->first;
            obj->resultbuffer += ": ";
            obj->resultbuffer += iter->second;
            obj->resultbuffer += "\r\n";
        }
    }
    obj->resultbuffer += "\r\n";

    obj->resultbuffer += obj->body.c_str();

    return 0;
}

int url_mux_flush_buffer(void* inst)
{
    url_mux_tag_t* obj = (url_mux_tag_t*)inst;
    if( !obj )
        return -1;

    obj->body.clear();
    obj->headers.cleaar();

    return 0;
}

int url_mux_free(void* inst)
{
    url_mux_tag_t* obj = (url_mux_tag_t*)inst;
    if( !obj )
        return -1;

    delete obj;
    return 0;    
}