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

#include <string>

#include "../../protocol/librtsp/rtspdemux.h"
#include "../../protocol/librtp/rtph264.h"
#include "../../file/libh26x/h264demux.h"
#include "rtspserver.h"

typedef struct 
{
    std::string client_ip;
    int client_rtp_port;  
    unsigned int session_id;
    st_netfd_t client_fd;
    int local_rtp_sock;
    int local_rtcp_sock;
    int local_rtp_port;
    int local_rtcp_port;
}rtsp_session_desc_t;

int url_getmsg(std::string &buffer, std::string &msg)
{
    int pos = buffer.find("\r\n\r\n");
    if( pos < 0 )
        return -1;

    msg = buffer.substr(0,pos+4);
    buffer.erase(0,pos+4);
    return 0;
}

void* mediaproc(void *arg)
{
    rtsp_session_desc_t* session = (rtsp_session_desc_t*)arg;
    int clientport = session->client_rtp_port;
    printf(" clientport = %d \n", clientport);

    st_netfd_t srv_nfd;
    if ((srv_nfd = st_netfd_open_socket(session->local_rtp_sock)) == NULL) {
      printf("st_netfd_open error");
      return NULL;
    }

    struct sockaddr_in server;  
    int addrlen =sizeof(server);  
    server.sin_family=AF_INET;  
    server.sin_port=htons(clientport);            
    server.sin_addr.s_addr = inet_addr(session->client_ip.c_str());  
    if( 0 > st_connect(srv_nfd, (const sockaddr *)&server, addrlen, ST_UTIME_NO_TIMEOUT) )
    {
      printf("st_connect error");
      return NULL;      
    }

    ////////////////////////////////////////////////
    void* rtphandle = rtp_mux_init(1);
    void* h264handle = H264Demux_Init((char*)"./test.264", 1);
    if( !h264handle )
    {
      printf("H264Framer_Init error\n");
      return NULL;
    }
    H264Configuration_t config;
    if( H264Demux_GetConfig(h264handle, &config) < 0 )
    {
      printf("H264Demux_GetConfig error\n");
      return NULL;
    }
    printf("H264Demux_GetConfig:width %d height %d framerate %d timescale %d %d %d \n",
        config.width, config.height, config.framerate, config.timescale,
        config.spslen, config.ppslen);

    const char *h264frame = NULL;
    int framelength = -1;
    unsigned int timestamp = 0;

    std::string temp_frame;

    const char* rtp_buffer = NULL;
    int rtp_packet_length, last_rtp_packet_length, rtp_packet_count;

    while( 1 )
    {
        int ret = H264Demux_GetFrame(h264handle, &h264frame, &framelength);
        if( ret < 0 )
        {
          printf("ReadOneNaluFromBuf error\n");
          break;
        }
        int frametype = h264frame[4]&0x1f;
        if( frametype == 5 )
        {
            H264Demux_GetConfig(h264handle, &config);
            temp_frame.clear();
            temp_frame.append(config.sps+4, config.spslen-4);
            temp_frame.append(config.pps, config.ppslen);
            temp_frame.append(h264frame, framelength);
        }
        else
        {
            temp_frame.clear();
            temp_frame.append(h264frame+4, framelength-4);
        }

//        rtp_set_h264_frame_over_udp(rtphandle, h264frame+4, framelength-4);
        rtp_set_h264_frame_over_udp(rtphandle, temp_frame.data(), temp_frame.size());
        rtp_get_h264_packet_over_udp(rtphandle, &rtp_buffer, &rtp_packet_length, &last_rtp_packet_length, &rtp_packet_count);

        for( int i = 0;i < rtp_packet_count;i++ )
        {
            if( i == rtp_packet_count - 1)
            {
                ret = st_write(srv_nfd, rtp_buffer, last_rtp_packet_length, ST_UTIME_NO_TIMEOUT);
            }
            else
            {
                ret = st_write(srv_nfd, rtp_buffer, rtp_packet_length, ST_UTIME_NO_TIMEOUT);
            }

            if( ret < 0 )
            {
              printf("st_write error\n");
              break;
            }
            rtp_buffer += rtp_packet_length;
        }

        st_usleep(1000 * 40);
    }
  st_netfd_close(srv_nfd);

  return NULL;
}

void *handle_request(void *arg) {

    rtsp_session_desc_t* session = (rtsp_session_desc_t*)arg;
    st_netfd_t cli_nfd = session->client_fd;
    printf("handle_request\n");

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    /* 获取本端的socket地址 */
//    nRet = getsockname(cli_nfd->osfd,(struct sockaddr*)&addr,&addr_len);
    getpeername(st_netfd_fileno(cli_nfd),(struct sockaddr*)&addr,&addr_len);
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
      memset(buf,0,sizeof(buf));
      int nr = (int) st_read(cli_nfd, buf, sizeof(buf), ST_UTIME_NO_TIMEOUT);
      if (nr <= 0)
        break;

      buffer.append(buf,nr);

      if( 0 != url_getmsg(buffer, msg) )
        continue;

      printf("msg\n%s\n", msg.c_str());
      int canSend = 0;
      std::string response = rtsp_demux_parse(rtspdemuxhandle, msg.c_str(), canSend);
//      std::string response = rtsp_parse(buf,canSend);
      if( response != "")
      {
        printf("response\n%s\n",response.c_str());
        int nw = st_write(cli_nfd, response.c_str(), response.size(), ST_UTIME_NO_TIMEOUT);
        if (nw <= 0)
          break;
      }

      if( canSend )
      {
        printf("canSend\n");
        session->client_rtp_port = rtsp_demux_get_client_rtp_port(rtspdemuxhandle);
        if (st_thread_create(mediaproc, session, 0, 0) == NULL) {
          printf("st_thread_create error");
          return NULL;
        }
      }
    }

  st_netfd_close(cli_nfd);
}

void *rtspserv(void *arg) {

  int port = *(int*)arg;
  /* Create and bind listening socket */
  int sock;
  if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    printf("socket error");
    return NULL;
  }
  int n = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&n, sizeof(n)) < 0) {
    printf("setsockopt error");
    return NULL;
  }
  struct sockaddr_in lcl_addr;
  bzero(&lcl_addr,sizeof(lcl_addr));
  lcl_addr.sin_family = AF_INET;
  lcl_addr.sin_addr.s_addr = htons(INADDR_ANY);
  lcl_addr.sin_port = htons(port);
  if (bind(sock, (struct sockaddr *)&lcl_addr, sizeof(lcl_addr)) < 0) {
    printf("bind error");
    return NULL;
  }
  listen(sock, 128);
  st_netfd_t srv_nfd;
  if ((srv_nfd = st_netfd_open_socket(sock)) == NULL) {
    printf("st_netfd_open error");
    return NULL;
  }

  unsigned int session_id = 100;
  st_netfd_t cli_nfd;
  struct sockaddr_in cli_addr;
  for ( ; ; ) {
    n = sizeof(cli_addr);
    cli_nfd = st_accept(srv_nfd, (struct sockaddr *)&cli_addr, &n,
     ST_UTIME_NO_TIMEOUT);
    if (cli_nfd == NULL) {
      printf("st_accept error");
      return NULL;
    }

    rtsp_session_desc_t* session = new rtsp_session_desc_t;
    session->session_id = session_id++;
    session->client_fd = cli_nfd;
    if (st_thread_create(handle_request, session, 0, 0) == NULL) {
      printf("st_thread_create error");
      return NULL;
    }
  }

  return 0;
}
