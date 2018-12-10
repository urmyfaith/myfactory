#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <sys/wait.h>

#include <fstream>

#include "mdemux_dmi/mdemux_dmi.h"

void *handle = NULL;
std::fstream fs;

void MyMCodecDemuxFinished(uint8_t *data, int size, const char *codec_name, uint32_t PTS, uint64_t UTS, void *user_data)
{
/*    int64_t millisecs = UTS % 1000;
    UTS /= 1000;
    char* timebuf = ctime(&UTS);
    printf("%s %d\n", timebuf, millisecs);
*/
    printf("myframecallback is being called==================== %d %s %d %lld\n", 
        size, codec_name, PTS, UTS);

   fs.write((char*)data, size);	
}

void signalproc(int sig)
{
    mdemux_dmi_free(handle);

    printf("signal exit \n");

    if( fs.is_open() )
        fs.close();

    exit(0);
//    return NULL;
}

int pullstream(char* streamid)
{
    signal(SIGINT, signalproc);

    fs.open("./111.h264", std::ios::out | std::ios::binary);

    std::string serverIP = "192.168.1.27";
    MResult ret = mdemux_dmi_alloc(&handle, (char*)serverIP.c_str(), 8905, (char*)"admin", (char*)"admin", (char*)"pullstream", NULL);
    if( !handle )
    {
        printf("mdemux_dmi_alloc error \n");
        return -1;
    }

    mdemux_dmi_setframecallback(handle, MyMCodecDemuxFinished);

    ret = mdemux_dmi_pullstream(handle, (char*)"live", streamid);
    if( ret.status < 0 )
    {
        printf("mdemux_dmi_pullstream error \n");
        return -1;
    }

    DMIPAYLOADTYPE type = mdemux_dmi_payloadtype(handle);
    printf("payloadtype = %d \n", type);

    mdemux_dmi_eventloop(handle);
    
    return 0;
}

int main(int argc, char *argv[])
{
    if( argc < 2 )
    {
        printf("parameter error\n");
        return -1;
    }
    
    pullstream(argv[1]);

    return 0;
}
