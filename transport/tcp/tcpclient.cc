#include   <sys/stat.h>     
#include   <sys/types.h>     
#include   <sys/socket.h>     
#include   <stdio.h>     
#include   <malloc.h>     
#include   <netdb.h>     
#include   <fcntl.h>  
#include   <unistd.h>  
#include   <netinet/in.h>  
#include   <arpa/inet.h>  
#include  <string.h>

#include "tcpclient.h"

struct tcpclient_t {
    int sockfd;

};

/************************************************************ 
 * 连接SOCKET服务器，如果出错返回-1，否则返回socket处理代码 
 * server：服务器地址(域名或者IP),serverport：端口 
 * ********************************************************/  
void* tcp_client_new(const char * server,int serverPort)
{  
    int    sockfd=0;  
    struct    sockaddr_in    addr;  
    struct    hostent        * phost;  
    //向系统注册，通知系统建立一个通信端口  
    //AF_INET表示使用IPv4协议  
    //SOCK_STREAM表示使用TCP协议  
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){  
        herror("Init socket error!");  
        return NULL;  
    }  
    bzero(&addr,sizeof(addr));  
    addr.sin_family = AF_INET;  
    addr.sin_port = htons(serverPort);  
    addr.sin_addr.s_addr = inet_addr(server);//按IP初始化  
      
    if(addr.sin_addr.s_addr == INADDR_NONE){//如果输入的是域名  
        phost = (struct hostent*)gethostbyname(server);  
        if(phost==NULL){  
            herror("Init socket s_addr error!");  
            return NULL;  
        }  
        addr.sin_addr.s_addr =((struct in_addr*)phost->h_addr)->s_addr;  
    }  
    if(connect(sockfd,(struct sockaddr*)&addr, sizeof(addr))<0)  
    {  
        perror("Connect server fail!");  
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
int tcp_client_write(void* handle, const char * sendBuff, int length)  
{  
    tcpclient_t * client = (tcpclient_t*)handle;

    int sendSize=send(client->sockfd,sendBuff, length,0);
    if(  sendSize <= 0){  
        herror("Send msg error!");  
        return -1;  
    }else  
        return sendSize;  
}  

/**************************************************************** 
 *接受消息，如果出错返回NULL，否则返回接受字符串的指针(动态分配，注意释放) 
 *sockfd：socket标识 
 * *********************************************************/  
int tcp_client_read(void* handle, char* buffer, int length)
{  
    tcpclient_t * client = (tcpclient_t*)handle;

    int recLenth=recv(client->sockfd, buffer, length,0);
    if( recLenth <= 0 )  
    {  
        return -1;
    }  

    return recLenth;  
}  
/************************************************** 
 *关闭连接 
 * **********************************************/  
int tcp_client_free(void* handle)  
{  
    tcpclient_t* client = (tcpclient_t*)handle;

    close(client->sockfd);  
    delete client;

    return 0;  
}  