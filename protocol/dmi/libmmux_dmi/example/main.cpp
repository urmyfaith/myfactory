#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <sys/wait.h>
#include <pthread.h>

#include <fstream>

#include "mmux_dmi/mmux_dmi.h"
#include "h264framer.h"

void *handle = NULL;
char *filepath = NULL;
int opencount = 0;
pthread_t id_pushstreamproc = 0;  

char buffer[1 * 1024 * 1024] = {0};
H264FileDesc_t desc;

void MySpeedXCallback(char* streamid, float speed, void* usrdata)
{
    printf("MySpeedXCallback is called %s %f\n", streamid, speed);
}

void MySeekCallback(char* streamid, char* duration, void* usrdata)
{
    printf("MySeekCallback is called %s %s\n", streamid, duration);
}

void* pushstreamproc(void* param)  
{
    desc.fs.open(filepath, std::ios::in|std::ios::binary);
    H264Framer_Init(desc);

    while( 1 )
    {
        pthread_testcancel();

        int ret = H264Framer_GetFrame(desc, buffer, sizeof(buffer));
        if( ret < 0 )
        {
//          println("frame data is nil")

            ret = H264Framer_ReadData(desc);
            if( ret < 0 ) 
            {
                printf("readdata error \n");
                break;
            }
            continue;
        }

//        printf("framelength = %d \n", ret);
        MResult res = mmux_dmi_sendframe(handle, buffer, ret, CODEC_H264, 0);
        if( res.status < 0 ) 
        {
            printf("mmux_dmi_sendframe error \n");
            break;
        }

        usleep(1000 * 1000 / desc.FPS);
    }

    pthread_exit(0);  
}

void MyPlayerOpenCallback(char* streamid, char* sessionid, void* usrdata)
{
    printf("MyPlayerOpenCallback is called %s %s\n", streamid, sessionid);
    opencount++;
    if( opencount == 1 )
    {
        int ret = pthread_create(&id_pushstreamproc,NULL, pushstreamproc,handle);  
        if(ret!=0)  
        {
            perror("Create decode thread error!\n");  
            return ;  
        }       
    }
}

void MyPlayerCloseCallback(char* streamid, char* sessionid, void* usrdata)
{
    printf("MyPlayerCloseCallback is called %s %s\n", streamid, sessionid);
    opencount--;
    if( opencount == 0 )
    {
        if( id_pushstreamproc > 0 )
        {
            pthread_cancel(id_pushstreamproc);
            pthread_join(id_pushstreamproc, NULL);
            id_pushstreamproc = 0;
        }
    }
}

void signalproc(int sig)
{
    mmux_dmi_free(handle);

    printf("signal exit \n");

    exit(0);
//    return NULL;
}

int pushstream(char *filepath)
{
    signal(SIGINT, signalproc);

    std::string serverIP = "192.168.1.27";
    MResult ret = mmux_dmi_alloc(&handle, (char*)serverIP.c_str(), 8905, (char*)"admin", (char*)"admin", (char*)"pullstream", NULL);
    if( !handle )
    {
        printf("mdemux_dmi_alloc error \n");
        return -1;
    }

    mmux_dmi_setspeedxcallback(handle, MySpeedXCallback);
    mmux_dmi_setseekcallback(handle, MySeekCallback);
    mmux_dmi_setplayeropencallback(handle, MyPlayerOpenCallback);
    mmux_dmi_setplayerclosecallback(handle, MyPlayerCloseCallback);

    std::string streamID = "t2";
    ret = mmux_dmi_pushstream(handle, (char*)"live", (char*)streamID.c_str(), CODEC_H264);
    if( ret.status < 0 )
    {
        printf("mdemux_dmi_pullstream error \n");
        return -1;
    }

    mmux_dmi_eventloop(handle);
    
    return 0;
}

int main(int argc, char *argv[])
{
    if( argc < 2 )
    {
        printf("parameter error \n");
        return -1;
    }

    filepath = argv[1];

    pushstream(argv[1]);

    return 0;
}
