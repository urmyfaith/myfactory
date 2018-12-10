#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <sys/types.h>  
#include <unistd.h>  
#include <errno.h>

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <sys/socket.h>  
    #include <netinet/in.h>  
    #include <arpa/inet.h>  
#endif

#include "rtp.h"

int get_local_rtp_rtcp_port(int *rtp_sock, int *rtp_port, int *rtcp_sock, int *rtcp_port)
{
    for( int i = 11000;i<30000;i+=2)
    {
        int rtpsock = socket(PF_INET, SOCK_DGRAM, 0);
        if( rtpsock < 0 ) 
        {
          printf("socket error");
          return -1;
        }

        struct sockaddr_in lcl_addr;
        memset(&lcl_addr, 0, sizeof(lcl_addr));
        lcl_addr.sin_family = AF_INET;
        lcl_addr.sin_addr.s_addr = htons(INADDR_ANY);
        lcl_addr.sin_port = htons(i);
        if (bind(rtpsock, (struct sockaddr *)&lcl_addr, sizeof(lcl_addr)) < 0) 
        {
            printf("bind error");
            close(rtpsock);
            continue;
        }
        *rtp_sock = rtpsock;
        *rtp_port = i;

        int rtcpsock = socket(PF_INET, SOCK_DGRAM, 0);
        if( rtcpsock < 0 ) 
        {
          printf("socket error");
          return -1;
        }

        lcl_addr.sin_port = htons(i+1);
        if (bind(rtcpsock, (struct sockaddr *)&lcl_addr, sizeof(lcl_addr)) < 0) 
        {
            printf("bind error");
            close(rtcpsock);
            close(rtpsock);
            continue;
        }
        *rtcp_sock = rtcpsock;
        *rtcp_port = i+1;

        break;
    }

    return 0;
}