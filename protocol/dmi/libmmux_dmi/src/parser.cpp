#include <stdio.h>  
#include <stdlib.h>  

#include "utils.h"
#include "parser.h"  

int ParseStatusCode(char *response)
{
    if( !response ) return -1;

    std::string strTemp = response;
    int pos = strTemp.find(" ");
    if( pos < 0 ) return -1;

    int pos1 = strTemp.find(" ", pos+1);
    if( pos1 < 0 ) return -1;

    return atoi(strTemp.substr(pos+1, pos1-pos-1).c_str());
}

int ParseRequestLine(char* response, std::string &method, std::string &url, std::string &version)
{
    if( !response ) return -1;

    std::string strTemp = response;
    int pos = strTemp.find(" ");
    if( pos < 0 ) return -1;
    method = strTemp.substr(0, pos);

    int pos1 = strTemp.find(" ", pos+1);
    if( pos1 < 0 ) return -1;
    url = strTemp.substr(pos+1, pos1-pos-1);    

    pos = pos1+1;
    pos1 = strTemp.find("\r\n", pos);
    if( pos1 < 0 ) return -1;
    version = strTemp.substr(pos, pos1-pos);

    return 0;
}

int ParseHeader(char* response, char *name, std::string &value)
{
    if( !response ) return -1;

    std::string strTemp = response;
    int pos = strTemp.find(name);
    if( pos < 0 ) return -1;

    int pos1 = strTemp.find("\r\n", pos);
    if( pos1 < 0 ) return -1;
    value = strTemp.substr(pos+strlen(name), pos1-pos-strlen(name));

    return 0;
}

int DMIDHeader_UnMarshal(DMIDHeader_t* header, char *buffer)
{
    memcpy(header, buffer, sizeof(DMIDHeader_t));

    header->UTS = __ntohll2(header->UTS); 
    header->PTS = ntohl(header->PTS); 

    header->PacketId = ntohs(header->PacketId); 
    header->PacketCount = ntohs(header->PacketCount); 

    header->SpeedX = ntohs(header->SpeedX); 
    header->PayloadSize = ntohs(header->PayloadSize); 

    return 0;
}
