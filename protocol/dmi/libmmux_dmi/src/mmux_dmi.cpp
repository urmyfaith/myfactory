#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/timeb.h>

#include "mmux_dmi.h"
#include "packer.h"
#include "parser.h"
#include "utils.h"

MResult mmux_dmi_alloc(void **handle, char *server_ip, int server_port, const char* user_name, const char* password, const char* user_agent, void* user_data)
{
    MResult ret;
    if( !server_ip || server_port <= 0 || !handle)
    {
        ret.status = -1;
        ret.error = "parameter error";
        return ret;
    }

    struct hostent *he = NULL;
    if((he = gethostbyname(server_ip)) == NULL || !handle)
    {
        perror("gethostbyname error");

        ret.status = -1;
        ret.error = "gethostbyname error";
        return ret;
    }
 
    int sockfd = -1;
    //new socket handle
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket error");

        ret.status = -1;
        ret.error = "socket error";
        return ret;
    }

    int flag = 1;
    int result = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
    if( result < 0 )
    {
        perror("setsockopt error");

        ret.status = -1;
        ret.error = "setsockopt error";
        return ret;
    } 
    
    struct sockaddr_in server;
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);
    server.sin_addr = *((struct in_addr *)he->h_addr);
     
    //connect server
    if(connect(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        ret.status = -1;
        ret.error = "connect() error";
        return ret;
    }

	DMIInstance_t *pInst = new DMIInstance_t;
	if( !pInst )
    {
        ret.status = -1;
        ret.error = "alloc instance error";
        return ret;
    }

	pInst->usrData = user_data;
	pInst->sockfd = sockfd;
    pInst->info.serverip = server_ip;
    pInst->info.serverport = server_port;
    pInst->info.username = user_name;
    pInst->info.password = password;
    pInst->info.useragent = user_agent;

    pInst->speedxcallback = NULL;
    pInst->seekcallback = NULL;
    pInst->playeropencallback = NULL;
    pInst->playerclosecallback = NULL;
    pInst->is_video = -1;
	
    *handle = (void*)(pInst);

    ret.status = 1;
    return ret;
}

MResult mmux_dmi_setspeedxcallback(void* handle, MCodecMuxSpeedXCallback callback)
{
    MResult ret;
    if( !handle )
    {
        ret.status = -1;
        ret.error = "parameter error";
        return ret;
    }

    DMIInstance_t *pInst = (DMIInstance_t*)handle;
    pInst->speedxcallback = callback;

    ret.status = 1;
    return ret;
}

MResult mmux_dmi_setseekcallback(void* handle, MCodecMuxSeekCallback callback)
{
    MResult ret;
    if( !handle )
    {
        ret.status = -1;
        ret.error = "parameter error";
        return ret;
    }

    DMIInstance_t *pInst = (DMIInstance_t*)handle;
    pInst->seekcallback = callback;

    ret.status = 1;
    return ret;
}

MResult mmux_dmi_setplayeropencallback(void* handle, MCodecMuxPlayerOpenCallback callback)
{
    MResult ret;
    if( !handle )
    {
        ret.status = -1;
        ret.error = "parameter error";
        return ret;
    }

    DMIInstance_t *pInst = (DMIInstance_t*)handle;
    pInst->playeropencallback = callback;

    ret.status = 1;
    return ret;
}

MResult mmux_dmi_setplayerclosecallback(void* handle, MCodecMuxPlayerCloseCallback callback)
{
    MResult ret;
    if( !handle )
    {
        ret.status = -1;
        ret.error = "parameter error";
        return ret;
    }

    DMIInstance_t *pInst = (DMIInstance_t*)handle;
    pInst->playerclosecallback = callback;

    ret.status = 1;
    return ret;
}

MResult mmux_dmi_setvideomediainfo(void* handle, VideoMediaInfo *info)
{
    MResult ret;
    if( !handle )
    {
        ret.status = -1;
        ret.error = "parameter error";
        return ret;
    }

    DMIInstance_t *pInst = (DMIInstance_t*)handle;
    pInst->is_video = 1;
    pInst->video_info_.sps.clear();
    pInst->video_info_.pps.clear();
    pInst->video_info_.vps.clear();
    pInst->video_info_.sps.append(info->sps, info->sps_length);
    pInst->video_info_.pps.append(info->pps, info->pps_length);
    pInst->video_info_.vps.append(info->vps, info->vps_length);
    pInst->video_info_.width = info->width;
    pInst->video_info_.height = info->height;

    ret.status = 1;
    return ret;
}

MResult mmux_dmi_setaudiomediainfo(void* handle, AudioMediaInfo *info)
{
    MResult ret;
    if( !handle )
    {
        ret.status = -1;
        ret.error = "parameter error";
        return ret;
    }

    DMIInstance_t *pInst = (DMIInstance_t*)handle;
    pInst->is_video = 0;
    pInst->audio_info_.sample_rate = info->sample_rate;
    pInst->audio_info_.bits_per_sample = info->bits_per_sample;
    pInst->audio_info_.channels = info->channels;

    ret.status = 1;
    return ret;
}

int readlen(DMIInstance_t *pInst, int length)
{
    char databuf[MAXDATASIZE] = {0};
    pInst->netbuffer.clear();

    while( length > 0 )
    {
        int recvlen = length > MAXDATASIZE ? MAXDATASIZE : length;
        int num = recv(pInst->sockfd, databuf, recvlen, 0);
        if( num < 0 )
        {
            printf("recv() error \n");
            return -1;
        }        
        length -= num;
        pInst->netbuffer.append(databuf, num);
    }

    return 0;
}

int readpayload(DMIInstance_t *pInst, DMIDHeader_t* header, int length)
{
    char databuf[MAXDATASIZE] = {0};
    while( length > 0 )
    {
        int recvlen = length > MAXDATASIZE ? MAXDATASIZE : length;
        int num = recv(pInst->sockfd, databuf, recvlen, 0);
        if( num < 0 )
        {
            printf("recv() error \n");
            return -1;
        }        
        length -= num;
        pInst->framebuffer.append(databuf, num);
    }

    return 0;
}

MResult mmux_dmi_pushstream(void* handle, char* stream_type, char* stream_id, DMIPAYLOADTYPE payloadtype)
{
    MResult ret;
    if( !handle )
    {
        ret.status = -1;
        ret.error = "parameter error";
        return ret;
    }

 	DMIInstance_t *pInst = (DMIInstance_t*)handle;
 	pInst->streamID = stream_id;
    pInst->info.streamtype = stream_type;

    std::string strrequest = GetRequestLine(pInst->info, "PUSH", pInst->streamID.c_str(), 2);
    strrequest += GetHeader("CSeq", "2");
    strrequest += GetHeader("User-Agent", pInst->info.useragent.c_str());
    strrequest += GetHeader("Codec", payloadtype);
    if( pInst->is_video == 1 ) 
    {
        strrequest += GetHeader("SPS", pInst->video_info_.sps.data());
        strrequest += GetHeader("PPS", pInst->video_info_.pps.data());
        strrequest += GetHeader("VPS", pInst->video_info_.vps.data());
        strrequest += GetHeader("Width", pInst->video_info_.width);
        strrequest += GetHeader("Height", pInst->video_info_.height);
    }
    else if( pInst->is_video == 0 )
    {
        strrequest += GetHeader("SampleRate", pInst->audio_info_.sample_rate);
        strrequest += GetHeader("BitsPerSample", pInst->audio_info_.bits_per_sample);
        strrequest += GetHeader("AudioChannel", pInst->audio_info_.channels);
    }
    strrequest += CRLF;

//    std::string strrequest = GetPushRequest(pInst->info, stream_id, 2, payloadtype);
    printf("request \n%s", strrequest.c_str());
    char buf[MAXDATASIZE] = {0};
    int packetlen = DMIDRequest_Marshal(2, (char*)strrequest.c_str(), strrequest.size(), buf, sizeof(buf));
    if( packetlen < 0 )
    {
        ret.status = -1;
        ret.error = "GetPullRequest error";
        return ret;
    }

    int result = send(pInst->sockfd, buf, packetlen, 0);
    if( result < 0 )
    {
        printf("send error:%d \n", errno);                
        perror("");

        ret.status = -1;
        ret.error = "send error";
        return ret;
    }  

    DMIDHeader_t header;
    result = readlen(pInst, 4+sizeof(header));
    if( result < 0 )
    {
        ret.status = -1;
        ret.error = "readlen error";
        return ret;
    }   
        
    DMIDHeader_UnMarshal(&header, (char*)pInst->netbuffer.data()+4);
        
//        printf("codec=%d, packetcount=%d, payloadsize = %d \n", header.Codec, header.PacketCount, header.PayloadSize);
    pInst->framebuffer.clear();

    result = readpayload(pInst, &header, header.PayloadSize);
    if( result < 0 )
    {
        ret.status = -1;
        ret.error = "readframe error";
        return ret;
    }   
    
    printf("response \n%s", pInst->framebuffer.c_str());
    if( 200 != ParseStatusCode((char*)pInst->framebuffer.c_str()) )
    {
        ret.status = -1;
        ret.error = "response error";
        return ret;
    }
    if( ParseHeader((char*)pInst->framebuffer.c_str(), (char*)"Session: ", pInst->sessionID) < 0 )
    {
        ret.status = -1;
        ret.error = "parse sessionID error";
        return ret;
    }

    ret.status = 1;
    return ret;
}

MResult mmux_dmi_eventloop(void* handle)
{
    MResult ret;
    if( !handle )
    {
        ret.status = -1;
        ret.error = "parameter error";
        return ret;
    }

//    mmux_dmi_report(handle, "mmux_dmi_eventloop");

    DMIInstance_t *pInst = (DMIInstance_t*)handle;
    DMIDHeader_t header;
    std::string strMethod, strURL, strVersion, strTemp;

    while( 1 )
    {
        int result = readlen(pInst, 4+sizeof(header));
        if( result < 0 )
        {
            ret.status = -1;
            ret.error = "readlen error";
            return ret;
        }   
        
        DMIDHeader_UnMarshal(&header, (char*)pInst->netbuffer.data()+4);
        
//        printf("codec=%d, packetcount=%d, payloadsize = %d \n", header.Codec, header.PacketCount, header.PayloadSize);
        if( header.PacketId == 0 )
            pInst->framebuffer.clear();

        result = readpayload(pInst, &header, header.PayloadSize);
        if( result < 0 )
        {
            ret.status = -1;
            ret.error = "readframe error";
            return ret;
        }
    
        if( header.PacketId == header.PacketCount - 1 )
        {
            switch( header.Codec )
            {
                case CODEC_DMIC_REQUEST:
                    printf("request %d\n%s", (int)pInst->framebuffer.size(), pInst->framebuffer.c_str());
                    if( ParseRequestLine((char*)pInst->framebuffer.c_str(), strMethod, strURL, strVersion) < 0 )
                    {
                        ret.status = -1;
                        ret.error = "ParseRequestLine error";
                        return ret;
                    }
//                    printf("%s %s %s\n", strMethod.c_str(), strURL.c_str(), strVersion.c_str());
                    if( strMethod == "PLAYER_OPEN" )
                    {
                        if( ParseHeader((char*)pInst->framebuffer.c_str(), (char*)"Session: ", strTemp) < 0 )
                        {
                            ret.status = -1;
                            ret.error = "ParseHeader error";
                            return ret;
                        }
//                        printf("Session = %s\n", strTemp.c_str());
                        if( pInst->playeropencallback )
                            pInst->playeropencallback((char*)pInst->streamID.c_str(), (char*)strTemp.c_str(), pInst->usrData);
                    }
                    else if( strMethod == "PLAYER_CLOSE" )
                    {
                        if( ParseHeader((char*)pInst->framebuffer.c_str(), (char*)"Session: ", strTemp) < 0 )
                        {
                            ret.status = -1;
                            ret.error = "ParseHeader error";
                            return ret;
                        }                        
//                        printf("Session = %s\n", strTemp.c_str());
                        if( pInst->playerclosecallback )
                            pInst->playerclosecallback((char*)pInst->streamID.c_str(), (char*)strTemp.c_str(), pInst->usrData);
                    }
                    else if( strMethod == "SEEK" )
                    {
                        if( ParseHeader((char*)pInst->framebuffer.c_str(), (char*)"Seek-PTS: ", strTemp) < 0 )
                        {
                            ret.status = -1;
                            ret.error = "ParseHeader error";
                            return ret;
                        }                        
//                        printf("Session = %s\n", strTemp.c_str());
                        if( pInst->seekcallback )
                            pInst->seekcallback((char*)pInst->streamID.c_str(), (char*)strTemp.c_str(), pInst->usrData);
                    }
                    else if( strMethod == "SPEEDX" )
                    {
                        if( ParseHeader((char*)pInst->framebuffer.c_str(), (char*)"SpeedX: ", strTemp) < 0 )
                        {
                            ret.status = -1;
                            ret.error = "ParseHeader error";
                            return ret;
                        }                        
//                        printf("Session = %s\n", strTemp.c_str());
                        if( pInst->speedxcallback )
                            pInst->speedxcallback((char*)pInst->streamID.c_str(), atof(strTemp.c_str()), pInst->usrData);
                    }

                    break;
                case CODEC_DMIC_RESPONSE:
                    printf("response \n%s", pInst->framebuffer.c_str());
                    if( 200 != ParseStatusCode((char*)pInst->framebuffer.c_str()) )
                    {
                        ret.status = -1;
                        ret.error = "response error";
                        return ret;
                    }
                    if( ParseHeader((char*)pInst->framebuffer.c_str(), (char*)"Session: ", pInst->sessionID) < 0 )
                    {
                        ret.status = -1;
                        ret.error = "parse sessionID error";
                        return ret;
                    }
                    break;
                default:
                    printf("unknown request \n%s", pInst->framebuffer.c_str());
                    break;
            }
        }
   }

    ret.status = 1;
    return ret;
}

MResult mmux_dmi_sendframe(void* handle, char* frame_data, int frame_length, DMIPAYLOADTYPE payload_type, uint32_t timestamp)
{
    MResult ret;
    if( !handle || frame_length <= 0 || !frame_data )
    {
        ret.status = -1;
        ret.error = "parameter error";
        return ret;
    }

    char buf[MAXDATASIZE] = {0};
    DMIInstance_t *pInst = (DMIInstance_t*)handle;
    int packetlength = 1440;
    int packetcount = frame_length / packetlength + frame_length % packetlength > 0 ? 1:0;

    DMIDHeader_t header = {.Version=2, 
                        .UTS = __htonll2(getSystemTime()),
                        .PTS = htonl(timestamp),
                        .Codec=payload_type, 
                        .PacketCount=htons(packetcount)}; 

    int offset = 0, packetid = 0;

//    printf("timestamp = %d\n", dmidheader.timestamp);
    while( frame_length > 0 )
    {
        int payloadlen = 0;
        if( frame_length - packetlength < 0 )
            payloadlen = frame_length;
        else
            payloadlen = packetlength;

        header.PayloadSize = htons(payloadlen);
        header.PacketId = htons(packetid);
        
        int buflen = DMIDRequest_Marshal(&header, frame_data+offset, payloadlen, buf, sizeof(buf));
        if( buflen < 0 )
        {
            ret.status = -1;
            ret.error = "DMIDRequest_Marshal error";
            return ret;
        }  

//        printf("packetlength = %d \n", buflen);
        int result = send(pInst->sockfd, buf, buflen, 0);
        if( result < 0 )
        {
            printf("send error:%d \n", errno);                
            perror("");

            ret.status = -1;
            ret.error = "send error";
            return ret;
        }  

        offset += packetlength;
        frame_length -= packetlength;

        packetid++;
    }

    ret.status = 1;
    return ret;
}

MResult mmux_dmi_report(void* handle, const char* infomation)
{
    MResult ret;
    if( !handle )
    {
        ret.status = -1;
        ret.error = "parameter error";
        return ret;
    }

    DMIInstance_t *pInst = (DMIInstance_t*)handle;

    std::string strrequest = GetRequestLine(pInst->info, "REPORT", pInst->streamID.c_str(), 2);
    strrequest += GetHeader("CSeq", "0");
    strrequest += GetHeader("User-Agent", pInst->info.useragent.c_str());
    strrequest += GetHeader("Session", pInst->sessionID.c_str());
    strrequest += GetHeader("Report", infomation);
    strrequest += CRLF;

//    std::string strrequest = GetReportRequest(pInst->info, (char*)pInst->streamID.c_str(), (char*)pInst->sessionID.c_str(), infomation, 2);
    char buf[MAXDATASIZE] = {0};
    int packetlen = DMIDRequest_Marshal(2, (char*)strrequest.c_str(), strrequest.size(), buf, sizeof(buf));
    if( packetlen < 0 )
    {
        ret.status = -1;
        ret.error = "GetPullRequest error";
        return ret;        
    }

    int result = send(pInst->sockfd, buf, packetlen, 0);
    if( result < 0 )
    {
        printf("send error:%d \n", errno);                
        perror("");
        ret.status = -1;
        ret.error = "send error";
        return ret;        
    }  

    ret.status = 1;
    return ret;
}

MResult mmux_dmi_free(void *handle)
{
    MResult ret;
	if( !handle )
	{
        ret.status = -1;
        ret.error = "parameter error";
		return ret;
	}
	
 	DMIInstance_t *pInst = (DMIInstance_t*)handle;

    close(pInst->sockfd);		
    delete pInst;

    ret.status = 1;
	return ret;
}
