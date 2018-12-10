#ifndef __TYPESDEFINED_H__
#define __TYPESDEFINED_H__

#include <stdint.h>
#include <string>
#include <pthread.h>
#include <deque>
#include <map>

#include "mdemux_dmi.h"

#define DATABUFFERSIZE 2 * 1024 * 1024
#define MAXDATASIZE 1024 * 2

#define CRLF "\r\n"

#pragma pack(push, 1)
typedef struct {  
	uint8_t Version;
	uint64_t UTS;    
	uint32_t PTS;       
	uint8_t Codec;     
	uint16_t PacketId;
	uint16_t PacketCount;
	float SpeedX;
	uint16_t PayloadSize;
} DMIDHeader_t; 
#pragma pack(pop)

typedef struct {  
    std::string serverip;
    uint32_t serverport;
    std::string username;
    std::string password;
    std::string streamtype;
    std::string useragent;
} DMICInfo_t; 

typedef struct 
{
	int sockfd;
	DMICInfo_t info;
	DMIDHeader_t header;
    std::string streamID;
    std::string sessionID;
    std::string streamType;

    MCodecDemuxFinished fcallback;
				
    std::string netbuffer;
    std::string framebuffer;

    DMIPAYLOADTYPE payloadtype;
	void* usrData;
}DMIInstance_t;

#endif
