#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>
#include <sys/select.h>
#include <sys/timeb.h>

#include "utils.h" 

int64_t  getSystemTime()
{
    struct timeb t;
    ftime(&t);
    return 1000 * t.time + t.millitm;
}

int is_little_endian(void)
{
  unsigned short flag=0x4321;
  if (*(unsigned char*)&flag==0x21)
    return 1;
  else
    return 0;
}

uint64_t __ntohll2(uint64_t val)
{
    if( is_little_endian() )
    {
        return (((uint64_t)htonl((int32_t)((val << 32) >> 32))) << 32) | (uint32_t)htonl((int32_t)(val >> 32));
    }
    else
    {
        return val;
    }
}

uint64_t __htonll2(uint64_t val)
{
    if( is_little_endian() )
    {
        return (((uint64_t)htonl((int32_t)((val << 32) >> 32))) << 32) | (uint32_t)htonl((int32_t)(val >> 32));
    }
    else
    {
        return val;
    }
}