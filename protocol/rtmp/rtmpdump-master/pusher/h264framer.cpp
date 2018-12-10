#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <errno.h>

#include "h264framer.h"  

int ParsePacket(char *buffer, int buflength, int &framelength, int &startpos) 
{
    int nFirstPos = -1;
    int i = 0;
    for( i = 0; i < buflength-4; i++)
    {
        if ( buffer[i] == 0 && buffer[i+1] == 0 &&
             ( buffer[i+2] == 1 || 
                ( buffer[i+2] == 0 && buffer[i+3] == 1 ) ) ) 
        {
            nFirstPos = i;
            i++;
            break;
        }
    }
    i++;
    if( nFirstPos < 0 )
    {
//      println("nFirstPos < 0 ")
        return -1;
    }

    int nEndPos = -1;
    for( ; i < buflength-4; i++)
    {
        if ( buffer[i] == 0 && buffer[i+1] == 0 &&
             (buffer[i+2] == 1 || (buffer[i+2] == 0 && buffer[i+3] == 1) ) ) {
            nEndPos = i;
            i++;
            break;
        }
    }
    if( nEndPos < 0)
    {
//      println("nEndPos < 0 ")
        return -1;
    }

    startpos = nFirstPos;
    framelength = nEndPos - nFirstPos;

    return 0;
}

int SetData(H264FileDesc_t &desc, char* data, int datalength)
{
    if( desc.offset + datalength > sizeof(desc.databuffer)) 
    {
        printf("databuffer overflow \n");
        return -1;
    }

//    printf("offset = %d \n", desc.offset);

    memcpy(desc.databuffer+desc.offset, data, datalength);
    desc.offset += datalength;

    return 0;
}

///////////////////////////////////////////////////////////
int H264Framer_GetFrame(H264FileDesc_t &desc, char *buffer, int buflength)
{
    int framelength = -1, startpos = -1;
    int ret = ParsePacket(desc.databuffer, desc.offset, framelength, startpos);
    if( ret < 0 ) return -1;

    if( framelength > buflength )
    {
        printf("buffer length is too short \n");
        return -1;
    }

    memcpy(buffer, desc.databuffer+startpos, framelength);

    memmove(desc.databuffer, desc.databuffer+startpos+framelength, desc.offset - startpos - framelength);
    desc.offset -= startpos + framelength;

    return framelength;
}

int H264Framer_Init(H264FileDesc_t &desc) 
{
    desc.offset = 0;
    desc.FPS = 25;

    return 0;
}

int H264Framer_ReadData(H264FileDesc_t &desc)
{
    if( !desc.fs.is_open() ) 
    {
        printf("file is not opened \n");
        return -1;
    }

    desc.fs.read(desc.buffer, sizeof(desc.buffer));
    int length = desc.fs.gcount();
    if( length <= 0 )
    {
        printf("seek file to the beginning \n");
        desc.fs.clear();
        desc.fs.seekg(0, std::ios::beg);
 
         desc.fs.read(desc.buffer, sizeof(desc.buffer));    
        length = desc.fs.gcount();
    }

    if( length <= 0 )
    {
        printf("file read error \n");
        return -1;
    }

    return SetData(desc, desc.buffer, length);
}
