#ifdef _WIN32 
    #include <Winsock2.h>
#elif defined __APPLE__
#elif defined __ANDROID__
#elif defined __linux__
    #include <sys/socket.h>
    #include <sys/select.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif

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
#include <iostream>

#include "udpselectserver.h"

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
    on_recv_callback recv_callback;

    void* userdata;
    std::thread threadeventloop;

    UDPCLIENTMAP clients;
    fd_set fdread, fdwrite;
};

int setnonblocking(int sock)
{
#ifdef _WIN32
     unsigned long NonBlock = 1;   
     if(ioctlsocket(sock, FIONBIO, &NonBlock) == SOCKET_ERROR)   
     {   
         printf("ioctlsocket() failed with error %d\n", WSAGetLastError());   
         return -1;   
     }   
    return 0;
#else
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
    return 0;
#endif
}

int select_fds(void* handle)
{
    udpserverdesc_t * inst = (udpserverdesc_t*)handle;
    FD_ZERO (&inst->fdread);
    FD_ZERO (&inst->fdwrite);
    
    FD_SET (inst->sockfd, &inst->fdread);
    FD_SET (inst->sockfd, &inst->fdwrite);

    if( !inst->clients.empty() )
    {
        for( UDPCLIENTMAP::iterator iter = inst->clients.begin();
            iter != inst->clients.end();
            ++iter)
        {
            FD_SET (iter->second->sockfd, &inst->fdread);        
            FD_SET (iter->second->sockfd, &inst->fdwrite);        
        }        
    }

    struct timeval tout;
    tout.tv_sec = 0;
    tout.tv_usec = 1000 * 500;
    int tag = select (inst->clients.size() + 1 + 1, &inst->fdread, &inst->fdwrite, NULL, &tout);
    if (tag == 0)
        printf("select wait timeout !");

    return tag;
}

int fdread_is_selected(void* handle, int fd)
{
    udpserverdesc_t * inst = (udpserverdesc_t*)handle;
    return FD_ISSET (fd, &inst->fdread);
}

int fdwrite_is_selected(void* handle, int fd)
{
    udpserverdesc_t * inst = (udpserverdesc_t*)handle;
    return FD_ISSET (fd, &inst->fdwrite);
}

int udp_select_server_eventloop(void* handle)
{
    udpserverdesc_t * inst = (udpserverdesc_t*)handle;

    struct sockaddr_in clientaddr;
    struct sockaddr_in serveraddr;
    int listenfd = socket(AF_INET, SOCK_DGRAM, 0);
    printf("listenfd = %d \n", listenfd);

    /*设置socket属性，端口可以重用*/  
    int opt=SO_REUSEADDR;  
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const char*)&opt,sizeof(opt));  

    //把socket设置为非阻塞方式
//    setnonblocking(listenfd);

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
#ifdef _WIN32
    serveraddr.sin_addr.s_addr =inet_addr(inst->localip.c_str());
#else
    inet_aton(inst->localip.c_str(),&(serveraddr.sin_addr));//htons(portnumber);
#endif

    serveraddr.sin_port=htons(inst->localport);
    bind(listenfd,(sockaddr *)&serveraddr, sizeof(serveraddr));
    listen(listenfd, 1024);
    inst->sockfd =listenfd;

    char buffer[1024 * 2] = {0};
    
    while( 1 ) 
    {
        usleep(1000 * 1);
        int nfds = select_fds(handle);

        if( fdread_is_selected(handle, listenfd) )
        {
            struct sockaddr_in client_addr;  
            int addr_size = sizeof(client_addr);  
            int ret = recvfrom(listenfd, buffer,sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_size);  
            if( ret <= 0 )
            {
                perror("recvfrom error");
                break;
            }        
            std::string clientip = inet_ntoa(client_addr.sin_addr);
            int clientport = ntohs(client_addr.sin_port);
//            std::cout << "recved from " << buf<< ret << std::endl;

            if( inst->recv_callback )
                inst->recv_callback(inst, buffer, ret, listenfd, (char*)clientip.c_str(), clientport, inst->userdata);
        }
    }

    return 0;
}

////////////////////////////////////////////////////////
void* udp_select_server_new(const char* localip, int localport, on_recv_callback recv_callback, void* userdata)
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif  /*  WIN32  */

    udpserverdesc_t * inst = new udpserverdesc_t;
    if( !inst ) return NULL;

    inst->localip = localip;
    inst->localport = localport;
    inst->recv_callback = recv_callback;
    inst->userdata = userdata;

    inst->threadeventloop = std::thread(udp_select_server_eventloop, (void*)inst);

    return inst;
}

int udp_select_server_write(int localfd, const char* data, int length, char* remoteip, int remoteport)
{
    struct    sockaddr_in    addr;  
    memset(&addr, 0, sizeof(addr));  
    addr.sin_family = AF_INET;  
    addr.sin_port = htons(remoteport);  
    addr.sin_addr.s_addr = inet_addr(remoteip);//按IP初始化  

    int addr_size = sizeof(addr);  
    int sendSize=sendto(localfd, data, length, 0, (const sockaddr*)&addr, addr_size);
    if(  sendSize < 0)
    {
        perror("udp_select_server_write error");
        return -1;
    } else 
        return sendSize;
}

int udp_select_server_free(void* handle)
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