#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <map>
#include <string>
#include <string.h>
#include <mutex>
#include <thread>

#include "tcpserver.h"

struct tcpclientdesc_t {
    int sockfd;
    std::string ip;
    int port;
    std::mutex recvmutex;
    std::mutex sendmutex;

    std::string recvbuffer;
    std::string sendbuffer, tempsendbuffer;    

    int status;
};

typedef std::map<int, tcpclientdesc_t*> TCPCLIENTMAP;

struct tcpserverdesc_t {
    int sockfd;
    std::string localip;
    int localport;
    on_connect_callback connectcallback;
    on_close_callback closecallback;

    void* userdata;
    std::thread threadeventloop;

    TCPCLIENTMAP clients;
    std::mutex clientsmutex;

    volatile int stopped;
};

int setnonblocking(int sock)
{
    int opts = fcntl(sock,F_GETFL);
    if(opts<0)
    {
        perror("fcntl(sock,GETFL)");
        return -1;
    }
    opts = opts|O_NONBLOCK;
    if(fcntl(sock,F_SETFL,opts)<0)
    {
        perror("fcntl(sock,SETFL,opts)");
        return -1;
    }
}

int tcp_server_eventloop(void* handle)
{
    tcpserverdesc_t * inst = (tcpserverdesc_t*)handle;

    //声明epoll_event结构体的变量,ev用于注册事件,数组用于回传要处理的事件
    struct epoll_event ev,events[20];

    //生成用于处理accept的epoll专用的文件描述符
    int epfd=epoll_create(256);
    struct sockaddr_in clientaddr;
    struct sockaddr_in serveraddr;
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);

    //把socket设置为非阻塞方式
    setnonblocking(listenfd);

    //设置与要处理的事件相关的文件描述符
    ev.data.fd=listenfd;
    //设置要处理的事件类型
    ev.events=EPOLLIN|EPOLLOUT|EPOLLET;
//    ev.events=EPOLLIN;

    //注册epoll事件
    epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_aton(inst->localip.c_str(),&(serveraddr.sin_addr));//htons(portnumber);

    serveraddr.sin_port=htons(inst->localport);
    bind(listenfd,(sockaddr *)&serveraddr, sizeof(serveraddr));
    listen(listenfd, 1024);
    inst->sockfd =listenfd;

    char buffer[1024 * 5] = {0};

    while( !inst->stopped ) 
    {
        //等待epoll事件的发生
        int nfds=epoll_wait(epfd,events,20,500);

        //处理所发生的所有事件
        for(int i=0;i<nfds;++i)
        {
            if(events[i].data.fd==listenfd)//如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。
            {
                socklen_t clilen;
                int connfd = accept(listenfd,(sockaddr *)&clientaddr, &clilen);
                if(connfd<0){
                    perror("accept connfd<0 \n");
                    return -1;
                }
                setnonblocking(connfd);
                if( inst->connectcallback )
                    inst->connectcallback(inst, connfd, inst->userdata);

                char *str = inet_ntoa(clientaddr.sin_addr);
//                std::cout << "accapt a connection from " << str << std::endl;
/*
                tcpclientdesc_t *client = new tcpclientdesc_t;
                client->sockfd = connfd;
                client->ip = str;
                client->status = 0;
                inst->clients[connfd] = client;

                //设置用于读操作的文件描述符
                ev.data.fd=connfd;
                //设置用于注测的读操作事件
                ev.events=EPOLLIN|EPOLLOUT|EPOLLET;
//                ev.events=EPOLLIN;

                //注册ev
                epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);
*/            }
            else if(events[i].events&EPOLLIN)//如果是已经连接的用户，并且收到数据，那么进行读入。
            {
//                std::cout<<"read"<<std::endl;
                int sockfd = events[i].data.fd;
                if ( sockfd < 0)
                    continue;

                TCPCLIENTMAP::iterator iter = inst->clients.find(sockfd);
                if( iter == inst->clients.end() )
                    return -1;

                while( 1 )
                {
                    int n = read(sockfd, buffer, sizeof(buffer));
                    if (n < 0) 
                    {
                        if (errno != EAGAIN) 
                        {
                            if( inst->closecallback )
                                inst->closecallback(inst, sockfd, inst->userdata);
                        
                            close(sockfd);
                            events[i].data.fd = -1;
                            iter->second->status = -1;
                        }
                        break;
                    } else if (n == 0) {
                        if( inst->closecallback )
                            inst->closecallback(inst, sockfd, inst->userdata);

                        close(sockfd);
                        events[i].data.fd = -1;
                        iter->second->status = -1;
                        break;
                    }
                    buffer[n] = '\0';
//                    std::cout<<buffer<<n<<std::endl;

                    iter->second->recvmutex.lock();
                    iter->second->recvbuffer.append(buffer, n);
                    iter->second->recvmutex.unlock();
                }

                //设置用于写操作的文件描述符
                ev.data.fd=sockfd;
                //设置用于注测的写操作事件
                ev.events=EPOLLIN|EPOLLOUT|EPOLLET;
//                ev.events=EPOLLOUT;

                //修改sockfd上要处理的事件为EPOLLOUT
                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
            }
            else if(events[i].events&EPOLLOUT) // 如果有数据发送
            {
//                std::cout<<"write"<<std::endl;
                int sockfd = events[i].data.fd;
                TCPCLIENTMAP::iterator iter = inst->clients.find(sockfd);
                if( iter == inst->clients.end() )
                    return -1;
            
                iter->second->sendmutex.lock();
                iter->second->tempsendbuffer.clear();
                iter->second->tempsendbuffer.append(iter->second->sendbuffer.data(), iter->second->sendbuffer.size());
                iter->second->sendmutex.unlock();                

                while( iter->second->tempsendbuffer.size() > 0 )
                {
                    if( iter->second->tempsendbuffer.size() <= 1024 * 5 )
                    {
                        int writelen = write(sockfd, iter->second->tempsendbuffer.data(), iter->second->tempsendbuffer.size());                    

                        iter->second->tempsendbuffer.clear();
                    }
                    else
                    {
                        int writelen = write(sockfd, iter->second->tempsendbuffer.data(), 1024*5);                        
                        iter->second->tempsendbuffer.erase(0, 1024 * 5);
                    }
                }

                //设置用于读操作的文件描述符
                ev.data.fd=sockfd;
                //设置用于注测的读操作事件
                ev.events=EPOLLIN|EPOLLOUT|EPOLLET;
//                ev.events=EPOLLIN;

                //修改sockfd上要处理的事件为EPOLIN
                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
            }
        }
    }

    close(epfd);
    return 0;
}

void* tcp_server_new(const char* localip, int localport, on_connect_callback connectcallback, on_close_callback closecallback, void* userdata)
{
    tcpserverdesc_t * inst = new tcpserverdesc_t;
    if( !inst ) return NULL;

    inst->localip = localip;
    inst->localport = localport;
    inst->connectcallback = connectcallback;
    inst->closecallback = closecallback;
    inst->userdata = userdata;
    inst->stopped = 0;

    inst->threadeventloop = std::thread(tcp_server_eventloop, (void*)inst);

    return inst;
}

int tcp_server_read(void* handle, int clientid, char* data, int length)
{
    tcpserverdesc_t * inst = (tcpserverdesc_t*)handle;

    int ret = read(clientid, data, length);
    //closed by peer
    if( ret == 0 && errno == EAGAIN )
    {
        printf("tcp_server_read error:socket reset by peer \n");  
        return -1;
    }

    if( ret < 0 )
    {  
//        printf("tcp_server_read ret = %d \n", ret);
        //no data received
        if( errno == EAGAIN || errno == EINTR )
            return 0;

        perror("tcp_server_read error:");  
        return -1;  
    }
    return ret;
/*    
    TCPCLIENTMAP::iterator iter = inst->clients.find(clientid);
    if( iter == inst->clients.end() )
        return -1;

    if( iter->second->status < 0 )
        return -1;

    int reallength = 0;
    iter->second->recvmutex.lock();
    if( iter->second->recvbuffer.size() <= 0 )
    {
        reallength = 0;
    }
    else if( iter->second->recvbuffer.size() <= length)
    {
        reallength = iter->second->recvbuffer.size();
        memcpy(data, iter->second->recvbuffer.data(), iter->second->recvbuffer.size());
        iter->second->recvbuffer.clear();        
    }
    else
    {
        reallength = length;
        memcpy(data, iter->second->recvbuffer.data(), length);
        iter->second->recvbuffer.erase(0, length);                
    }
    iter->second->recvmutex.unlock();

    return reallength;
*/
}

int tcp_server_write(void* handle, int clientid, const char* data, int length)
{
    tcpserverdesc_t * inst = (tcpserverdesc_t*)handle;

    int ret = write(clientid, data, length);
//    printf("writelen=%d, length=%d \n", writelen, length);
    while( ret <= 0 )
    {  
        if( errno != EAGAIN && errno != EINTR )
        {
            perror("tcp_server_write error:");  
            return -1;  
        }
        else if( ret == 0 && errno == EAGAIN )
        {
            perror("tcp_server_write error:");  
            return -1;  
        }

        ret = write(clientid, data, length);
    }
    return ret;

/*
    TCPCLIENTMAP::iterator iter = inst->clients.find(clientid);
    if( iter == inst->clients.end() )
        return -1;

    if( iter->second->status < 0 )
        return -1;

    iter->second->sendmutex.lock();
    iter->second->sendbuffer.append(data, length);
    iter->second->sendmutex.unlock();

    return 0;
*/
}

int tcp_server_close(void* handle, int clientid)
{
    tcpserverdesc_t * inst = (tcpserverdesc_t*)handle;

    close(clientid);
    return 0;

/*
    TCPCLIENTMAP::iterator iter = inst->clients.find(clientid);
    if( iter == inst->clients.end() )
        return -1;

    close(clientid);
    tcpclientdesc_t* client = iter->second;
    delete client;
    inst->clients.erase(iter);

    return 0;
*/
}

int tcp_server_free(void* handle)
{
    tcpserverdesc_t * inst = (tcpserverdesc_t*)handle;
    inst->stopped = 1;

    if( !inst->clients.empty() )
    {
        for( TCPCLIENTMAP::iterator iter = inst->clients.begin();
            iter != inst->clients.end();
            ++iter)
            {
                close(iter->first);
                tcpclientdesc_t* client = iter->second;
                delete client;
            }                
    }

    close(inst->sockfd);
    delete inst;

    return 0;
}