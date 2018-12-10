#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <string>

#include "stringbuffer.h"  

typedef struct {

    std::string databuffer;
}StringBufferDesc_t;

///////////////////////////////////////////////////////////
void* stringbuffer_alloc(int initialsize) 
{
    StringBufferDesc_t* desc = new StringBufferDesc_t;
    if( !desc )
        return NULL;

    if( initialsize > 0)
        desc->databuffer.reserve(initialsize);

    return desc;
}

uint32_t stringbuffer_getsize(void *handle)
{
    StringBufferDesc_t *desc = (StringBufferDesc_t*)handle;
    return desc->databuffer.size();
}

char* stringbuffer_getbuffer(void *handle)
{
    StringBufferDesc_t *desc = (StringBufferDesc_t*)handle;
    return desc->databuffer.data();
}

int stringbuffer_append(void *handle, const char* data, int length)
{
    StringBufferDesc_t *desc = (StringBufferDesc_t*)handle;
    desc->databuffer.append(data, length);
    return 0;
}

int stringbuffer_erasefromhead(void *handle, int length)
{
    StringBufferDesc_t *desc = (StringBufferDesc_t*)handle;
    desc->databuffer.erase(0, length);
    return 0;
}

int stringbuffer_free(void *handle)
{
    StringBufferDesc_t *desc = (StringBufferDesc_t*)handle;
    delete desc;
    return 0;
}
