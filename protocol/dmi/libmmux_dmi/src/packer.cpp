#include <stdio.h>  
#include <stdlib.h>  
#include <sstream>
#include <arpa/inet.h>

#include "packer.h"  
#include "utils.h"

std::string GetRequestLine(DMICInfo_t &dmic, const char* method, const char* streamid, int version)
{
    std::stringstream ss;

    ss<<method<<" dmi://"<<dmic.username<<":"<<dmic.password<<"@"<<dmic.serverip<<":"<<dmic.serverport<<"/"<<dmic.streamtype<<"/"<<streamid<<" DMI/"<<version<<".0"<<CRLF;

    return ss.str().c_str();
}

std::string GetHeader(const char* headername, const char* headervalue)
{
    std::stringstream ss;

    ss<<headername<<": "<<headervalue<<CRLF;

    return ss.str().c_str();
}

std::string GetHeader(const char* headername, float headervalue)
{
    std::stringstream ss;

    ss<<headername<<": "<<headervalue<<CRLF;

    return ss.str().c_str();
}

std::string GetHeader(const char* headername, int headervalue)
{
    std::stringstream ss;

    ss<<headername<<": "<<headervalue<<CRLF;

    return ss.str().c_str();
}

int DMIDRequest_Marshal(int version, char* payload, int payloadsize, char *buffer, int bufsize)
{
    DMIDHeader_t header = {.Version=version, 
                        .Codec=CODEC_DMIC_REQUEST, 
                        .PacketId=htons(0), 
                        .PacketCount=htons(1), 
                        .PayloadSize=htons(payloadsize)}; 
//    printf("%d, %d \n", header.PayloadSize, header.Version);

    if( bufsize < 4+sizeof(header)+payloadsize )
    {
        return -1;
    }

    memcpy(buffer, (char*)"DMID", strlen("DMID"));
    memcpy(buffer+4, &header, sizeof(header));
    memcpy(buffer+4+sizeof(header), payload, payloadsize);

    return 4+sizeof(header)+payloadsize;
}

int DMIDRequest_Marshal(DMIDHeader_t *header, char* payload, int payloadsize, char *buffer, int bufsize)
{
    if( bufsize < 4+sizeof(*header)+payloadsize )
    {
        return -1;
    }

    memcpy(buffer, (char*)"DMID", strlen("DMID"));
    memcpy(buffer+4, header, sizeof(*header));
    memcpy(buffer+4+sizeof(*header), payload, payloadsize);

    return 4+sizeof(*header)+payloadsize;
}
