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

#include "mdemux_dmi.h"
#include "packer.h"
#include "parser.h"
#include "utils.h"

MResult mdemux_dmi_alloc(void **handle, char *server_ip, int server_port, const char* user_name, const char* password, const char* user_agent, void* user_data)
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
        perror("gethostbyname error:");

        ret.status = -1;
        ret.error = "gethostbyname error";
        return ret;
    }
 
    int sockfd = -1;
    //new socket handle
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket error:");

        ret.status = -1;
        ret.error = "socket error";
        return ret;
    }

    int flag = 1;
    int result = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
    if( result < 0 )
    {
        perror("setsockopt error:");
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
        printf("connect() error\n");
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

    pInst->fcallback = NULL;
    pInst->payloadtype = CODEC_INVALID;
	
    *handle = (void*)(pInst);

    ret.status = 1;
    return ret;
}

MResult mdemux_dmi_setframecallback(void* handle, MCodecDemuxFinished fcallback)
{
    MResult ret;
    if( !handle )
    {
        ret.status = -1;
        ret.error = "parameter error";
        return ret;
    }

    DMIInstance_t *pInst = (DMIInstance_t*)handle;
    pInst->fcallback = fcallback;

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
            printf("recv() error\n");
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
            printf("recv() error\n");
            return -1;
        }        
        length -= num;
        pInst->framebuffer.append(databuf, num);
    }

    return 0;
}

MResult mdemux_dmi_pullstream(void* handle, char* stream_type, char* stream_id)
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

    std::string strrequest = GetRequestLine(pInst->info, "PULL", stream_id, 2);
    strrequest += GetHeader("CSeq", "2");
    strrequest += GetHeader("User-Agent", pInst->info.useragent.c_str());
    strrequest += CRLF;

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
    if( ParseHeader((char*)pInst->framebuffer.c_str(), (char*)"Codec: ", strrequest) < 0 )
    {
        ret.status = -1;
        ret.error = "parse Codec error";
        return ret;
    }
    pInst->payloadtype = (DMIPAYLOADTYPE)atoi(strrequest.c_str());

    ret.status = 1;
    return ret;
}

DMIPAYLOADTYPE mdemux_dmi_payloadtype(void* handle)
{
    if( !handle )
    {
        return CODEC_UNKNOWN;
    }

    DMIInstance_t *pInst = (DMIInstance_t*)handle;
    return  pInst->payloadtype;
}

MResult mdemux_dmi_eventloop(void* handle)
{
    MResult ret;
    if( !handle )
    {
        ret.status = -1;
        ret.error = "parameter error";
        return ret;
    }

    DMIInstance_t *pInst = (DMIInstance_t*)handle;
    DMIDHeader_t header;

//    mdemux_dmi_speedx(handle, 2.0);
//    mdemux_dmi_seek(handle, (char*)"21");
//    mdemux_dmi_report(handle, (char*)"mdemux_dmi_eventloop");

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
                    printf("request \n%s", pInst->framebuffer.c_str());
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
                case CODEC_UNKNOWN:
                    printf("unknown request \n%s", pInst->framebuffer.c_str());
                    break;
                default:
                    if( pInst->fcallback )
                    {
                        pInst->fcallback((uint8_t*)pInst->framebuffer.data(), pInst->framebuffer.size(), getpayloadtypestr((DMIPAYLOADTYPE)header.Codec), header.PTS, header.UTS, pInst->usrData);
                    }
                    break;
            }
        }
   } 

    ret.status = 1;
    return ret;
}

MResult mdemux_dmi_speedx(void* handle, float speed)
{
    MResult ret;
    if( !handle )
    {
        ret.status = -1;
        ret.error = "parameter error";
        return ret;
    }

    DMIInstance_t *pInst = (DMIInstance_t*)handle;

    std::string strrequest = GetRequestLine(pInst->info, "SPEEDX", pInst->streamID.c_str(), 2);
    strrequest += GetHeader("CSeq", "0");
    strrequest += GetHeader("User-Agent", pInst->info.useragent.c_str());
    strrequest += GetHeader("Session", pInst->sessionID.c_str());
    strrequest += GetHeader("SpeedX", speed);
    strrequest += CRLF;

    printf("request %d \n%s", (int)strrequest.size(), strrequest.c_str());
    char buf[MAXDATASIZE] = {0};
    int packetlen = DMIDRequest_Marshal(2, (char*)strrequest.c_str(), strrequest.size(), buf, sizeof(buf));
    if( packetlen < 0 )
    {
        ret.status = -1;
        ret.error = "GetSpeedXRequest error";
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

MResult mdemux_dmi_seek(void* handle, char* duration)
{
    MResult ret;
    if( !handle )
    {
        ret.status = -1;
        ret.error = "parameter error";
        return ret;
    }

    DMIInstance_t *pInst = (DMIInstance_t*)handle;

    std::string strrequest = GetRequestLine(pInst->info, "SEEK", pInst->streamID.c_str(), 2);
    strrequest += GetHeader("CSeq", "0");
    strrequest += GetHeader("User-Agent", pInst->info.useragent.c_str());
    strrequest += GetHeader("Session", pInst->sessionID.c_str());
    strrequest += GetHeader("Seek-PTS", duration);
    strrequest += CRLF;

    printf("request %d \n%s", (int)strrequest.size(), strrequest.c_str());
    char buf[MAXDATASIZE] = {0};
    int packetlen = DMIDRequest_Marshal(2, (char*)strrequest.c_str(), strrequest.size(), buf, sizeof(buf));
    if( packetlen < 0 )
    {
        ret.status = -1;
        ret.error = "GetSeekRequest error";
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

MResult mdemux_dmi_report(void* handle, char* infomation)
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

    char buf[MAXDATASIZE] = {0};
    int packetlen = DMIDRequest_Marshal(2, (char*)strrequest.c_str(), strrequest.size(), buf, sizeof(buf));
    if( packetlen < 0 )
    {
        ret.status = -1;
        ret.error = "GetReportRequest error";
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

MResult mdemux_dmi_free(void *handle)
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
