/*

  Copyright (c) 2015 Martin Sustrik

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <thread>
#include <string>

#include "../../protocol/librtsp/rtspdemux.h"
#include "../../protocol/librtp/rtp.h"
#include "../../transport/tcp/tcpserver.h"
#include "../../transport/udp/udpclient.h"

#include "rtspserver.h"
#include "demuxer.h"
#include "muxer.h"

typedef struct 
{
    std::string sourcename;
    std::string sourcetype;

    std::string client_ip;
    int client_rtp_port;  
    unsigned int session_id;
    int client_fd;
    int local_rtp_sock;
    int local_rtcp_sock;
    int local_rtp_port;
    int local_rtcp_port;
    void* handle;
}rtsp_session_desc_t;

#pragma pack(push, 1)
typedef struct
{ 
    uint8_t magic : 8;// $ 0x24
    uint8_t channel : 8; //0-1 0-video rtp,1-video rtcp,2-audio rtp,3-audio rtcp
    uint16_t rtp_len : 16;
}rtsp_interleaved;
#pragma pack(pop)

unsigned int session_id = 100;

void* tcpmediaproc(void *arg)
{
    rtsp_session_desc_t* session = (rtsp_session_desc_t*)arg;
    void* tcpserverhandle = session->handle;
    int client_fd = session->client_fd;

    rtsp_interleaved ri;
    ri.magic = 0x24;
    ri.channel = 0;

    ////////////////////////////////////////////////
    void *muxerhandle = muxer_alloc(session->sourcetype.c_str());

    printf("session->sourcename = %s \n", session->sourcename.c_str());
    void *framerhandle = demuxer_alloc(session->sourcename.c_str());

    while( 1 )
    {
        const char *h264frame = NULL;
        int framelength = -1;
        int ret = demuxer_getframe(framerhandle, &h264frame, &framelength);
        if( ret < 0 )
        {
            printf("framer_getframe error\n");
            break;
        }
        
        muxer_setframe(muxerhandle, h264frame, framelength);

        const char* rtp_buffer = NULL;
        int rtp_packet_length, last_rtp_packet_length, rtp_packet_count;
        muxer_getpacket(muxerhandle, &rtp_buffer, &rtp_packet_length, &last_rtp_packet_length, &rtp_packet_count);

        printf("framelength=%d rtp_packet_count=%d \n", framelength, rtp_packet_count);
        for( int i = 0;i < rtp_packet_count;i++ )
        {
            if( i == rtp_packet_count - 1)
            {
                ri.rtp_len = htons(last_rtp_packet_length);
                ret = tcp_server_write(tcpserverhandle, client_fd, (const char*)&ri, sizeof(ri));
                if( ret < 0 ){printf("tcp_server_write error\n");break;}
                ret = tcp_server_write(tcpserverhandle, client_fd, rtp_buffer, last_rtp_packet_length);
                if( ret < 0 ){printf("tcp_server_write error\n");break;}
            }
            else
            {
                ri.rtp_len = htons(rtp_packet_length);
                ret = tcp_server_write(tcpserverhandle, client_fd, (const char*)&ri, sizeof(ri));
                if( ret < 0 ){printf("tcp_server_write error\n");break;}
                ret = tcp_server_write(tcpserverhandle, client_fd, rtp_buffer, rtp_packet_length);
                if( ret < 0 ){printf("tcp_server_write error\n");break;}
            }

            rtp_buffer += rtp_packet_length;
        }

        if( ret < 0 )
        {
            break;
        }
        usleep(1000 * 40);//1000);
    }

    muxer_free(muxerhandle);
    demuxer_free(framerhandle);
    tcp_server_close(tcpserverhandle, client_fd);
    printf("tcpmediaproc exit \n");
    return NULL;
}

void* udpmediaproc(void *arg)
{
    rtsp_session_desc_t* session = (rtsp_session_desc_t*)arg;
    int clientport = session->client_rtp_port;
    printf(" clientport = %d \n", clientport);

    void* udphandle = udp_client_new(session->client_ip.c_str(), clientport, -1, session->local_rtp_sock);
    if( !udphandle )
    {
      printf("udp_client_new error \n");
      return NULL;
    }

    ////////////////////////////////////////////////
    void *muxerhandle = muxer_alloc(session->sourcetype.c_str());
    void *framerhandle = demuxer_alloc(session->sourcename.c_str());

    while( 1 )
    {
        const char *h264frame = NULL;
        int framelength = -1;
        int ret = demuxer_getframe(framerhandle, &h264frame, &framelength);
        if( ret < 0 )
        {
            printf("framer_getframe error\n");
            break;
        }

        muxer_setframe(muxerhandle, h264frame, framelength);

        const char* rtp_buffer = NULL;
        int rtp_packet_length, last_rtp_packet_length, rtp_packet_count;
        muxer_getpacket(muxerhandle, &rtp_buffer, &rtp_packet_length, &last_rtp_packet_length, &rtp_packet_count);

        for( int i = 0;i < rtp_packet_count;i++ )
        {
            if( i == rtp_packet_count - 1)
            {
                ret = udp_client_write(udphandle, rtp_buffer, last_rtp_packet_length);
            }
            else
            {
                ret = udp_client_write(udphandle, rtp_buffer, rtp_packet_length);
            }

            if( ret < 0 )
            {
              printf("udp_client_write error\n");
              break;
            }
            rtp_buffer += rtp_packet_length;
        }
        
        if( ret < 0 )
        {
            break;
        }
        usleep(1000 * 40);
    }
  
    udp_client_free(udphandle);
    muxer_free(muxerhandle);
    demuxer_free(framerhandle);
    printf("udpmediaproc exit \n");
    return NULL;
}

void *handle_request(void *arg) {

    rtsp_session_desc_t* session = (rtsp_session_desc_t*)arg;
    printf("handle_request\n");

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    /* 获取本端的socket地址 */
//    nRet = getsockname(cli_nfd->osfd,(struct sockaddr*)&addr,&addr_len);
    getpeername(session->client_fd,(struct sockaddr*)&addr,&addr_len);
    session->client_ip = inet_ntoa(addr.sin_addr);
    printf("clientip = %s \n", session->client_ip.c_str());

    if( 0 > get_local_rtp_rtcp_port(&session->local_rtp_sock, &session->local_rtp_port, 
                &session->local_rtcp_sock, &session->local_rtcp_port))
    {
        printf("get_local_rtp_rtcp_port error\n");
        return NULL;
    }

    char buf[1024] = {0};
    sprintf(buf, "%d", session->session_id);
    void* rtspdemuxhandle = rtsp_demux_init(buf, session->local_rtp_port);

    std::string buffer, msg;
    for(;;) {
      sleep(1);

      memset(buf,0,sizeof(buf));
      int nr = tcp_server_read(session->handle, session->client_fd, buf, sizeof(buf));
      if (nr < 0)
        break;

      buffer.append(buf,nr);
//      printf("buffer\n%s\n", buffer.c_str());

      if( 0 != url_getmsg(buffer, msg) )
        continue;

      printf("msg\n%s\n", msg.c_str());
      int canSend = 0, transport_proto = -1;
      const char* strResponse = NULL;
      int ret = rtsp_demux_parse(rtspdemuxhandle, msg.c_str(), &strResponse, transport_proto, canSend);
      if( ret < 0 )
        break;

      std::string response = strResponse;
      printf("response\n%s\n",response.c_str());
      int nw = tcp_server_write(session->handle, session->client_fd, (char*)response.c_str(), response.size());
      if (nw < 0)
        break;

      if( canSend )
      {
        printf("canSend\n");
        const char* filename = NULL;
        rtsp_demux_getfilename(rtspdemuxhandle, &filename);
        session->sourcename = filename;
        const char* filetype = NULL;
        rtsp_demux_getfileext(rtspdemuxhandle, &filetype);
        session->sourcetype = filetype;
        
        session->client_rtp_port = rtsp_demux_get_client_rtp_port(rtspdemuxhandle);
        if( transport_proto == 0 )//udp
        {
            std::thread t(udpmediaproc, (void*)session);
            t.detach();
        }
        else if( transport_proto == 1 )//tcp
        {
            std::thread t(tcpmediaproc, (void*)session);
            t.detach();
        }
      }
    }

    close(session->client_fd);
    delete session;
    printf("handle_request exit \n");
}

//////////////////////////////////////////////////////////
int myon_connect_callback(void* handle, int sockfd, void* userdata)
{
    printf("myon_connect_callback %d \n", sockfd);

    rtsp_session_desc_t* session = new rtsp_session_desc_t;
    session->session_id = session_id++;
    session->client_fd = sockfd;
    session->handle = handle;

    std::thread t(handle_request, (void*)session);
    t.detach();

    return 0;
}

int myon_close_callback(void* handle, int sockfd, void* userdata)
{
    printf("myon_close_callback %d \n", sockfd);
    
    tcp_server_close(handle, sockfd);

    return 0;
}

void *rtspserv(int port)
{
    return tcp_server_new("0.0.0.0", port, myon_connect_callback, myon_close_callback, NULL);
}

int rtspstop(void* handle)
{
    return tcp_server_free(handle);
}