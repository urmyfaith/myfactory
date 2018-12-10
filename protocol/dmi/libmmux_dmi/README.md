# DMISDK for C

## Overview
	
DMISDK is the implementation of dmi protocol(what is dmi?see the parts about DMI in the wiki).It provides C APIs to use.
	
## Build
1.build and install libjsoncpp first.  
Go to [libjsoncpp website](https://github.com/open-source-parsers/jsoncpp).

2.make

3.make install
	
## Usage

### Interfaces
<pre><code>
typedef enum 
{
    CODEC_H264 = 0,
    CODEC_H265 = 1,
    CODEC_VP8  = 2,
    CODEC_VP9  = 3,

    CODEC_MJPEG = 50,
    CODEC_PNG   = 51,

    CODEC_AAC  = 100,
    CODEC_G711 = 101,

    CODEC_JSON     = 150,
    CODEC_PROTOBUF = 151,
    CODEC_TEXT     = 152,

    CODEC_DMIC_REQUEST  = 200,
    CODEC_DMIC_RESPONSE = 201,

    CODEC_UNKNOWN = 255

}DMIPAYLOADTYPE;

/**
* description: play speed callback function
*
* return:
*
* @param streamid: streamid
* @param speed: play speed
* @param usrdata: callback usrdata
**/
typedef int (*SpeedXCallback)(char* streamid, float speed, void* usrdata);

/**
* description: play seek callback function
*
* return:
*
* @param streamid: streamid
* @param duration: seek duration
* @param usrdata: callback usrdata
**/
typedef int (*SeekCallback)(char* streamid, char* duration, void* usrdata);

/**
* description: player open callback function
*
* return:
*
* @param streamid: streamid
* @param sessionid: sessionid
* @param usrdata: callback usrdata
**/
typedef int (*PlayerOpenCallback)(char* streamid, char* sessionid, void* usrdata);

/**
* description: player close callback function
*
* return:
*
* @param streamid: streamid
* @param sessionid: sessionid
* @param usrdata: callback usrdata
**/
typedef int (*PlayerCloseCallback)(char* streamid, char* sessionid, void* usrdata);

/**
* alloc a new instance
*
* return: <0-error, =0 success
*
* @param serverIP: server ip
* @param serverPort: server port
* @param username: server username
* @param password: server password
* @param useragent: user defined agent name
* @param usrdata: callback usrdata
* @param handle: instance handle
**/
int mmux_dmi_alloc(char *serverIP, int serverPort, char* username, char* password, char* useragent, void* usrdata, void **handle);

/**
* play speed callback
*
* return: <0-error, =0 success
*
* @param handle: instance handle
* @param callback: play speed callback function
**/
int mmux_dmi_setspeedxcallback(void* handle, SpeedXCallback callback);

/**
* play seek callback
*
* return: <0-error, =0 success
*
* @param handle: instance handle
* @param callback: play seek callback function
**/
int mmux_dmi_setseekcallback(void* handle, SeekCallback callback);

/**
* player open callback
*
* return: <0-error, =0 success
*
* @param handle: instance handle
* @param callback: player open callback function
**/
int mmux_dmi_setplayeropencallback(void* handle, PlayerOpenCallback callback);

/**
* player close callback
*
* return: <0-error, =0 success
*
* @param handle: instance handle
* @param callback: player close callback function
**/
int mmux_dmi_setplayerclosecallback(void* handle, PlayerCloseCallback callback);

/**
* push video stream
*
* return: 0-success, <0-error
*
* @param handle: instance handle
* @param streamtype: live,vod,proxy
* @param streamid: stream id
**/
int mmux_dmi_pushstream(void* handle, char* streamtype, char* streamid);

/**
* dmi event loop
*
* return: 0-success, <0-error
*
* @param handle: instance handle
**/
int mmux_dmi_eventloop(void* handle);

/**
* send frame data
*
* return: 0-success, <0-error
*
* @param handle: instance handle
* @param framedata: framedata
* @param framelength: framelength
**/
int mmux_dmi_sendframe(void* handle, char* framedata, int framelength, DMIPAYLOADTYPE payloadtype, uint32_t timestamp);

/**
* destroy a instance
*
* return: 0-success, <0-error
*
* @param handle: instance handle
**/
int mmux_dmi_free(void *handle);
</code></pre>

### Example	
<pre><code>
void *handle = NULL;
char *filepath = NULL;
int opencount = 0;
pthread_t id_pushstreamproc = 0;  

char buffer[1 * 1024 * 1024] = {0};
H264FileDesc_t desc;

int MySpeedXCallback(char* streamid, float speed, void* usrdata)
{
    printf("MySpeedXCallback is called %s %f\n", streamid, speed);
    return 0;
}

int MySeekCallback(char* streamid, char* duration, void* usrdata)
{
    printf("MySeekCallback is called %s %s\n", streamid, duration);
    return 0;
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
        ret = mmux_dmi_sendframe(handle, buffer, ret, CODEC_H264, 0);
        if( ret < 0 ) 
        {
            printf("mmux_dmi_sendframe error \n");
            break;
        }

        usleep(1000 * 1000 / 1000);
//        usleep(1000 * 1000 / desc.FPS);
    }

    pthread_exit(0);  
}

int MyPlayerOpenCallback(char* streamid, char* sessionid, void* usrdata)
{
    printf("MyPlayerOpenCallback is called %s %s\n", streamid, sessionid);
    opencount++;
    if( opencount == 1 )
    {
        int ret = pthread_create(&id_pushstreamproc,NULL, pushstreamproc,handle);  
        if(ret!=0)  
        {
            perror("Create decode thread error!\n");  
            return -1;  
        }       
    }

    return 0;
}

int MyPlayerCloseCallback(char* streamid, char* sessionid, void* usrdata)
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

    return 0;
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
    int ret = mmux_dmi_alloc((char*)serverIP.c_str(), 8905, (char*)"admin", (char*)"admin", (char*)"pullstream", NULL, &handle);
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
    ret = mmux_dmi_pushstream(handle, (char*)"live", (char*)streamID.c_str());
    if( ret < 0 )
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
</code></pre>