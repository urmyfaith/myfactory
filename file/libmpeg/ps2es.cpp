#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <unistd.h>

#define BUFFERSIZE 1000 * 1024
char pInputBuffer[BUFFERSIZE] = {0};
#define PACKETSIZE 100 * 1024
char tempbuffer[PACKETSIZE] = {0};

std::fstream fsh264;
///////////////////////////////////////////////////////////
int  ParsePacket(char* pBuffer,int nBufferLen,char* pPesBuffer,int& nPesLen)
{
    int nReadPos=0;
    int nFirstPesPos=-1;
    int nPesEndPos=-1;
    char* p=pBuffer;

    // find first pes packet position
//    printf("nBufferLen=%u \n", nBufferLen);
    while (nReadPos < (nBufferLen-4) )
    {
//        printf("%02x ",p[nReadPos]);
        if ((unsigned char)(p[nReadPos])== (unsigned char)0 && 
            (unsigned char)(p[nReadPos+1])== (unsigned char)0 && 
            (unsigned char)(p[nReadPos+2])== (unsigned char)1 && 
            (unsigned char)(p[nReadPos+3])== (unsigned char)0xe0
            )
        {
             nFirstPesPos=nReadPos;
             nReadPos++;
             break;
        }
        nReadPos++;
    }

    if (nFirstPesPos<0)
    {
        printf("nFirstPesPos < 0 \n");
        return -1;
    }

    // find pes end pos
    while (nReadPos<nBufferLen-4)
    {
        if ((unsigned char)p[nReadPos]==(unsigned char)0 && 
            (unsigned char)p[nReadPos+1]== (unsigned char)0 && 
            (unsigned char)p[nReadPos+2]== (unsigned char)1 && 
            (
                (unsigned char)p[nReadPos+3]==(unsigned char)0xe0 || 
                (unsigned char)p[nReadPos+3]==(unsigned char)0xba || 
                (unsigned char)p[nReadPos+3]==(unsigned char)0xc0)
            )
        {
            nPesEndPos=nReadPos;
            nReadPos++;
            break;
        }
        nReadPos++;
    }

    if (nPesEndPos<0)
    {
        printf("nPesEndPos < 0 \n");
        return -1;
    }

    printf("start pos:%d end pos:%d \n",nFirstPesPos,nPesEndPos);
    nPesLen= nPesEndPos - nFirstPesPos;
    memcpy(pPesBuffer, p+nFirstPesPos, nPesLen);
//    fwrite(pPesBuffer,1,nPesLen,pFile);
    return nFirstPesPos;
}
 
 
//////////////////////////////////////////////////////////////////////////
///实时流回调
void SetPacket(char *pBuffer,uint dwBufSize)
{
    static uint nOfferSet = 0;
    //fwrite(pBuffer,dwBufSize,1,pFile);
    if ( nOfferSet > BUFFERSIZE )
    {
        printf("buffer overflow \n");
        return;
    }

    printf("input buffer size %d noffset:%d, %x\n",dwBufSize,nOfferSet, (char)pBuffer[0]);
    memcpy(pInputBuffer+nOfferSet, pBuffer, dwBufSize);
    nOfferSet+=(int)dwBufSize;

    int nPesOutLen = 0;
    int nRet = ParsePacket(pInputBuffer,nOfferSet,tempbuffer,nPesOutLen);
    while ( nRet>=0 )
    {
        unsigned pesheaderlen= 9 + (unsigned char)tempbuffer[8];
        fsh264.write( tempbuffer + pesheaderlen, nPesOutLen - pesheaderlen);
        memmove(pInputBuffer,pInputBuffer+nRet+nPesOutLen,nOfferSet-nRet-nPesOutLen);
        nOfferSet=nOfferSet-nRet-nPesOutLen;
        printf("##########  noffset:%d\n",nOfferSet);
        nRet = ParsePacket(pInputBuffer,nOfferSet,tempbuffer,nPesOutLen);
    }
}

int main()
{
    std::fstream fs;
    fs.open("./111.ps", std::ios::binary | std::ios::in);
    fsh264.open("./111.264", std::ios::binary | std::ios::out);

    if( !fs.is_open() || !fsh264.is_open() )
    {
        printf("file open failed \n");
        return -1;
    }

    while( 1 )
    {
        int tempbufferlen = sizeof(tempbuffer);
        fs.read(tempbuffer, tempbufferlen);
        int length = fs.gcount();
        if( length <= 0 )
            break;

//        if( (unsigned)tempbuffer[3] == 0xba )
//            printf("---------------------- \n");

//        printf("%01x %01x %01x %x \n", tempbuffer[0], tempbuffer[1], tempbuffer[2], (unsigned char)tempbuffer[3]);
//        break;

        SetPacket(tempbuffer, length);

//        usleep(100);
    }
    fs.close();    
    fsh264.close();

    return 0;
}