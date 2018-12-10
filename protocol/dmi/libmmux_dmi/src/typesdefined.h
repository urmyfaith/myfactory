#ifndef __TYPESDEFINED_H__
#define __TYPESDEFINED_H__

#include <stdint.h>
#include <string>
#include <pthread.h>
#include <deque>
#include <map>

#include "mmux_dmi.h"

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

typedef struct
{
	std::string vps;
	std::string sps;
	std::string pps;
	int width;
	int height;
}VideoMediaInfo_;

typedef struct
{
	int sample_rate;
	int bits_per_sample;
	int channels;
}AudioMediaInfo_;

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
				
	MCodecMuxSpeedXCallback speedxcallback;
	MCodecMuxSeekCallback seekcallback;
	MCodecMuxPlayerOpenCallback playeropencallback;
	MCodecMuxPlayerCloseCallback playerclosecallback;

	int is_video;
	VideoMediaInfo_ video_info_;
	AudioMediaInfo_ audio_info_;

    std::string netbuffer;
    std::string framebuffer;

	void* usrData;
}DMIInstance_t;

#endif
