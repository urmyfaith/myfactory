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
#include <deque>

#include "udpserver.h"

typedef std::deque<std::string> UDPCLIENTDATAQUEUE;

struct udpclientdesc_t {
    int sockfd;
    std::string ip;
    int port;
    std::mutex recvmutex;
    std::mutex sendmutex;

    UDPCLIENTDATAQUEUE recvqueue;
    UDPCLIENTDATAQUEUE sendqueue;
};

typedef std::map<int, udpclientdesc_t*> UDPCLIENTMAP;

struct udpserverdesc_t {
    int sockfd;
    std::string localip;
    int localport;
    on_connect_callback connectcallback;
    on_close_callback closecallback;

    void* userdata;
    std::thread threadeventloop;

    UDPCLIENTMAP clients;
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

int udp_socket_connect(void* handle, int epollfd,struct sockaddr_in  *client_addr)  
{        
    udpserverdesc_t * inst = (udpserverdesc_t*)handle;

    struct sockaddr_in my_addr, their_addr;  
    int fd=socket(PF_INET, SOCK_DGRAM, 0);  
      
    /*设置socket属性，端口可以重用*/  
    int opt=SO_REUSEADDR;  
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));  

    setnonblocking(fd);  
    bzero(&my_addr, sizeof(my_addr));  
    my_addr.sin_family = PF_INET;  
    my_addr.sin_port = htons(inst->localport);  
    my_addr.sin_addr.s_addr = INADDR_ANY;  
    if (bind(fd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1)   
    {  
        perror("bind");  
        return -1;
    }   
    else  
    {  
        printf("IP and port bind success \n");  
    }  

    if(fd==-1)  
        return  -1;  

    if( connect(fd,(struct sockaddr*)client_addr,sizeof(struct sockaddr_in)) < 0 )
    {
        perror("connect");  
        return -1;        
    }  

    return fd;     
}  
   
int accept_client(void* handle, int epollfd,int listenfd)  
{  
    udpserverdesc_t * inst = (udpserverdesc_t*)handle;

    struct sockaddr_in client_addr;  
    socklen_t addr_size = sizeof(client_addr);  
    char buf[2048] = {0};  
    while( 1 )
    {
        int ret = recvfrom(listenfd, buf,sizeof(buf), 0, (struct sockaddr *)&client_addr, &addr_size);  
        //check(ret > 0, "recvfrom error");  
        if( ret <= 0 )
        {
            perror("recvfrom error");
            break;;
        }        
        std::string clientip = inet_ntoa(client_addr.sin_addr);
        int clientport = ntohs(client_addr.sin_port);
        std::cout << "recved from " << buf<< ret << std::endl;

        if( !inst->clients.empty() )
        {
            UDPCLIENTMAP::iterator iter = inst->clients.begin();
            for( ;iter != inst->clients.end(); ++iter )
            {
                if( iter->second->ip == clientip &&
                    iter->second->port == clientport )
                {
                    iter->second->recvmutex.lock();
                    iter->second->recvqueue.push_back(std::string(buf, ret));
                    iter->second->recvmutex.unlock();

                    break;
                }
            }        
            if( iter != inst->clients.end() )
                continue;
        }

        std::cout << "accapt a connection from " << clientip<<" "<<clientport << std::endl;
        int new_sock=udp_socket_connect(handle, epollfd,(struct sockaddr_in*)&client_addr);  
          
        udpclientdesc_t *client = new udpclientdesc_t;
        client->recvqueue.push_back(std::string(buf, ret));
        client->sockfd = new_sock;
        client->ip = clientip;
        client->port = clientport;
        inst->clients[new_sock] = client;

        if( inst->connectcallback )
            inst->connectcallback(inst, new_sock, inst->userdata);

        //设置用于读操作的文件描述符
        struct epoll_event ev;
        ev.data.fd=new_sock;
        ev.events=EPOLLIN|EPOLLET;
        epoll_ctl(epollfd,EPOLL_CTL_ADD,new_sock,&ev);
    }

    return 0;  
}  

int udp_server_eventloop(void* handle)
{
    udpserverdesc_t * inst = (udpserverdesc_t*)handle;

    //声明epoll_event结构体的变量,ev用于注册事件,数组用于回传要处理的事件
    struct epoll_event ev,events[20];

    //生成用于处理accept的epoll专用的文件描述符
    int epfd=epoll_create(256);
    struct sockaddr_in clientaddr;
    struct sockaddr_in serveraddr;
    int listenfd = socket(AF_INET, SOCK_DGRAM, 0);
    printf("listenfd = %d \n", listenfd);

    /*设置socket属性，端口可以重用*/  
    int opt=SO_REUSEADDR;  
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));  

    //把socket设置为非阻塞方式
    setnonblocking(listenfd);

    //设置与要处理的事件相关的文件描述符
    ev.data.fd=listenfd;
    //设置要处理的事件类型
    ev.events=EPOLLIN|EPOLLET;
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

    char buffer[1024 * 2] = {0};

    while( 1 ) 
    {
        //等待epoll事件的发生
        int nfds=epoll_wait(epfd,events,20,500);
//        std::cout<<"epoll_wait "<<nfds<<std::endl;

        //处理所发生的所有事件
        for(int i=0;i<nfds;++i)
        {
            if(events[i].data.fd==listenfd)//如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。
            {
                accept_client(handle, epfd, listenfd);
            }
            else if(events[i].events&EPOLLIN)//如果是已经连接的用户，并且收到数据，那么进行读入。
            {
                std::cout<<"read"<<std::endl;
                int sockfd = events[i].data.fd;
                if ( sockfd < 0)
                    continue;

                UDPCLIENTMAP::iterator iter = inst->clients.find(sockfd);
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
                        }
                        break;
                    } else if (n == 0) {
                        if( inst->closecallback )
                            inst->closecallback(inst, sockfd, inst->userdata);

                        close(sockfd);
                        events[i].data.fd = -1;
                        break;
                    }
                    buffer[n] = '\0';
                    std::cout<<buffer<<n<<std::endl;

                    iter->second->recvmutex.lock();
                    iter->second->recvqueue.push_back(std::string(buffer, n));
                    iter->second->recvmutex.unlock();
                }

                //设置用于写操作的文件描述符
                ev.data.fd=sockfd;
                //设置用于注测的写操作事件
                ev.events=EPOLLOUT|EPOLLET;
//                ev.events=EPOLLOUT;

                //修改sockfd上要处理的事件为EPOLLOUT
                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
            }
            else if(events[i].events&EPOLLOUT) // 如果有数据发送
            {
                std::cout<<"write"<<std::endl;
                int sockfd = events[i].data.fd;
                UDPCLIENTMAP::iterator iter = inst->clients.find(sockfd);
                if( iter == inst->clients.end() )
                    return -1;
            
                iter->second->sendmutex.lock();
                if( iter->second->sendqueue.size() > 0 )
                {
                    std::string data = iter->second->sendqueue.front();
                    write(sockfd, data.data(), data.size());                    
                    iter->second->sendqueue.pop_front();
                }
                iter->second->sendmutex.unlock();

                //设置用于读操作的文件描述符
                ev.data.fd=sockfd;
                //设置用于注测的读操作事件
                ev.events=EPOLLIN|EPOLLET;
//                ev.events=EPOLLIN;

                //修改sockfd上要处理的事件为EPOLIN
                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
            }
        }
    }

    close(epfd);
    return 0;
}

void* udp_server_new(const char* localip, int localport, on_connect_callback connectcallback, on_close_callback closecallback, void* userdata)
{
    udpserverdesc_t * inst = new udpserverdesc_t;
    if( !inst ) return NULL;

    inst->localip = localip;
    inst->localport = localport;
    inst->connectcallback = connectcallback;
    inst->closecallback = closecallback;
    inst->userdata = userdata;

    inst->threadeventloop = std::thread(udp_server_eventloop, (void*)inst);

    return inst;
}

int udp_server_read(void* handle, int clientid, char* data, int length)
{
    udpserverdesc_t * inst = (udpserverdesc_t*)handle;

    UDPCLIENTMAP::iterator iter = inst->clients.find(clientid);
    if( iter == inst->clients.end() )
        return -1;

    int reallength = 0;
    iter->second->recvmutex.lock();
    if( iter->second->recvqueue.size() <= 0 )
    {
        reallength = 0;
    }
    else
    {
        std::string frontdata = iter->second->recvqueue.front();
        reallength = frontdata.size();
        memcpy(data, frontdata.data(), frontdata.size());
        iter->second->recvqueue.pop_front();
    }
    iter->second->recvmutex.unlock();

    return reallength;
}

int udp_server_write(void* handle, int clientid, const char* data, int length)
{
    udpserverdesc_t * inst = (udpserverdesc_t*)handle;

    UDPCLIENTMAP::iterator iter = inst->clients.find(clientid);
    if( iter == inst->clients.end() )
        return -1;

    iter->second->sendmutex.lock();
    iter->second->sendqueue.push_back(std::string(data, length));
    iter->second->sendmutex.unlock();

    return 0;
}

int udp_server_close(void* handle, int clientid)
{
    udpserverdesc_t * inst = (udpserverdesc_t*)handle;

    UDPCLIENTMAP::iterator iter = inst->clients.find(clientid);
    if( iter == inst->clients.end() )
        return -1;

    close(clientid);
    udpclientdesc_t* client = iter->second;
    delete client;
    inst->clients.erase(iter);

    return 0;
}

int udp_server_free(void* handle)
{
    udpserverdesc_t * inst = (udpserverdesc_t*)handle;

    if( !inst->clients.empty() )
    {
        for( UDPCLIENTMAP::iterator iter = inst->clients.begin();
            iter != inst->clients.end();
            ++iter)
            {
                close(iter->first);
                udpclientdesc_t* client = iter->second;
                delete client;
            }                
    }

    close(inst->sockfd);
    delete inst;

    return 0;
}