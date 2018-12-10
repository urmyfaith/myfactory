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

#include "udpclient.h"

struct tcpclient_t {
    int sockfd;

};

/************************************************************ 
 * 连接SOCKET服务器，如果出错返回-1，否则返回socket处理代码 
 * server：服务器地址(域名或者IP),serverport：端口 
 * ********************************************************/  
void* udp_client_new(const char * server,int serverPort, int localport, int localfd)
{  
    int    sockfd=0;  
    struct    sockaddr_in    addr;  
    struct    hostent        * phost;  
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

    memset(&addr, 0, sizeof(addr));  
    addr.sin_family = AF_INET;  
    addr.sin_port = htons(serverPort);  
    addr.sin_addr.s_addr = inet_addr(server);//按IP初始化  
      
    if(addr.sin_addr.s_addr == INADDR_NONE){//如果输入的是域名  
        phost = (struct hostent*)gethostbyname(server);  
        if(phost==NULL){  
            perror("Init socket s_addr error!");  
            return NULL;  
        }  
        addr.sin_addr.s_addr =((struct in_addr*)phost->h_addr)->s_addr;  
    }  
    if(connect(sockfd,(struct sockaddr*)&addr, sizeof(addr))<0)  
    {  
        perror("Connect server error");  
        return NULL; //0表示成功，-1表示失败  
    }  

    tcpclient_t * client = new tcpclient_t;
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
int udp_client_write(void* handle, const char * sendBuff, int length)  
{  
    tcpclient_t * client = (tcpclient_t*)handle;

    int sendSize=send(client->sockfd,sendBuff, length,0);
    if(  sendSize < 0)
    {  
//        printf("udp_client_write error:%d \n", errno);
        perror("udp_client_write error");
        return -1;
    } else 
        return sendSize;  
}  

/**************************************************************** 
 *接受消息，如果出错返回NULL，否则返回接受字符串的指针(动态分配，注意释放) 
 *sockfd：socket标识 
 * *********************************************************/  
int udp_client_read(void* handle, char* buffer, int length)
{  
    tcpclient_t * client = (tcpclient_t*)handle;

    int recLenth=recv(client->sockfd, buffer, length,0);
    if( recLenth < 0 )  
    {  
        perror("udp_client_read error");
        return -1;
    }  

    return recLenth;  
}  
/************************************************** 
 *关闭连接 
 * **********************************************/  
int udp_client_free(void* handle)  
{  
    tcpclient_t* client = (tcpclient_t*)handle;

    close(client->sockfd);  
    delete client;

    return 0;  
}