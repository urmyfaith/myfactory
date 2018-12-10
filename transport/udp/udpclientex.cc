#ifdef _WIN32
    #include <Winsock2.h>
#else
    #include   <sys/socket.h>     
    #include   <netdb.h>     
    #include   <netinet/in.h>  
    #include   <arpa/inet.h>  
#endif

#include   <sys/stat.h>     
#include   <sys/types.h>     

#include   <stdio.h>     
#include   <malloc.h>     
#include   <fcntl.h>  
#include   <unistd.h>  
#include  <string.h>
#include    <errno.h>
#include <string>

#include "udpclientex.h"

struct udpclientex_t {
    int sockfd;

    std::string remoteip;
};

int selectread_fd(int fd)
{
    fd_set fdread;
    FD_ZERO (&fdread);
    
    FD_SET (fd, &fdread);

    struct timeval tout;
    tout.tv_sec = 0;
    tout.tv_usec = 1000 * 500;
    int tag = select (2, &fdread, NULL, NULL, &tout);
    if (tag == 0)
        printf("selectread_fd: select wait timeout\n");

    return tag;
}
/////////////////////////////////////////
/************************************************************ 
 * 连接SOCKET服务器，如果出错返回-1，否则返回socket处理代码 
 * server：服务器地址(域名或者IP),serverport：端口 
 * ********************************************************/  
void* udp_clientex_new(int localport, int localfd)
{  
    int    sockfd=0;  
    //向系统注册，通知系统建立一个通信端口  
    //AF_INET表示使用IPv4协议  
    //SOCK_STREAM表示使用TCP协议  
    if( localfd > 0 )
        sockfd = localfd;
    else
    {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif  /*  WIN32  */

        if((sockfd=socket(AF_INET,SOCK_DGRAM,0))<0){  
            perror("Init socket error!");  
            return NULL;  
        }        
    }

    if( localport >= 0 )
    {
        struct sockaddr_in lcl_addr;
        memset(&lcl_addr, 0, sizeof(lcl_addr));
        lcl_addr.sin_family = AF_INET;
        lcl_addr.sin_addr.s_addr = htons(INADDR_ANY);
        lcl_addr.sin_port = htons(localport);
        if (bind(sockfd, (struct sockaddr *)&lcl_addr, sizeof(lcl_addr)) < 0) 
        {
            perror("bind error");
            close(sockfd);
            return NULL;
        }
    }

    udpclientex_t * client = new udpclientex_t;
    if( !client ){
        close(sockfd);
        return NULL;
    }
    client->sockfd = sockfd;
    return client;
}  

/************************************************************** 
 * 发送消息，如果出错返回-1，否则返回发送的字符长度 
 * sockfd：socket标识，sendBuff：发送的字符串 
 * *********************************************************/  
int udp_clientex_write(void* handle, const char * sendBuff, int length, 
        const char* remoteip, int remoteport)  
{  
    udpclientex_t * client = (udpclientex_t*)handle;

    struct    sockaddr_in    addr;  
    memset(&addr, 0, sizeof(addr));  
    addr.sin_family = AF_INET;  
    addr.sin_port = htons(remoteport);  
    addr.sin_addr.s_addr = inet_addr(remoteip);//按IP初始化  

    int addr_size = sizeof(addr);  
    int sendSize=sendto(client->sockfd, sendBuff, length, 0, (const sockaddr*)&addr, addr_size);
    if(  sendSize < 0)
    {
//        printf("udp_client_write error:%d \n", errno);
        perror("udp_client_write error");
        return -1;
    } else 
        return sendSize;  
}  

int udp_clientex_selectread(void* handle)
{
    udpclientex_t * client = (udpclientex_t*)handle;
    return selectread_fd(client->sockfd);
}

/**************************************************************** 
 *接受消息，如果出错返回NULL，否则返回接受字符串的指针(动态分配，注意释放) 
 *sockfd：socket标识 
 * *********************************************************/  
int udp_clientex_read(void* handle, char* buffer, int length,
    const char** remoteip, int *remoteport)
{
    udpclientex_t * client = (udpclientex_t*)handle;

    struct sockaddr_in addr;  
    int addr_size = sizeof(addr);  
    int recLenth = recvfrom(client->sockfd, buffer, length, 0, (struct sockaddr *)&addr, &addr_size);  
    if( recLenth < 0 )  
    {  
        perror("udp_client_read error");
        return -1;
    }  

    client->remoteip = inet_ntoa(addr.sin_addr);
    *remoteip = client->remoteip.c_str();
    *remoteport = ntohs(addr.sin_port);

    return recLenth;      
}
/************************************************** 
 *关闭连接 
 * **********************************************/  
int udp_clientex_free(void* handle)  
{  
    udpclientex_t* client = (udpclientex_t*)handle;

    close(client->sockfd);  
    delete client;

    return 0;  
}