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
* description: frame data callback function
*
* return:
*
* @param data: frame data buffer
* @param length: frame data length
* @param payloadtype: frame data type
* @param PTS: frame data timestamp
* @param UTS: frame data absolute timestammp signature in milliseconds
* @param usrdata: callback usrdata
**/
typedef int (*FrameCallback)(char *data, uint32_t length, DMIPAYLOADTYPE payloadtype, uint32_t PTS, uint64_t UTS, void* usrdata);

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
int mdemux_dmi_alloc(char *serverIP, int serverPort, char* username, char* password, char* useragent, void* usrdata, void **handle);

/**
* set frame data callback
*
* return: <0-error, =0 success
*
* @param handle: instance handle
* @param fcallback: frame data callback function
**/
int mdemux_dmi_setframecallback(void* handle, FrameCallback fcallback);

/**
* pull video stream
*
* return: 0-success, <0-error
*
* @param handle: instance handle
* @param streamtype: live,vod,proxy
* @param streamid: stream id
**/
int mdemux_dmi_pullstream(void* handle, char* streamtype, char* streamid);

/**
* dmi event loop
*
* return: 0-success, <0-error
*
* @param handle: instance handle
**/
int mdemux_dmi_eventloop(void* handle);

/**
* set video speed
*
* return: 0-success, <0-error
*
* @param handle: instance handle
* @param speed: 1/8,1/4,1/2,1,2,4,8
**/
int mdemux_dmi_speedx(void* handle, float speed);

/**
* set video play duration
*
* return: 0-success, <0-error
*
* @param handle: instance handle
* @param duration: play starttime
**/
int mdemux_dmi_seek(void* handle, char* duration);

/**
* report infomation user defined
*
* return: 0-success, <0-error
*
* @param handle: instance handle
* @param infomation: infomation as string
**/
int mdemux_dmi_report(void* handle, char* infomation);

/**
* destroy a instance
*
* return: 0-success, <0-error
*
* @param handle: instance handle
**/
int mdemux_dmi_free(void *handle);
</code></pre>

### Example	
<pre><code>
void *handle = NULL;
std::fstream fs;

int MyFrameCallback(char *data, uint32_t length, DMIPAYLOADTYPE payloadtype, uint32_t PTS, uint64_t UTS, void* usrdata)
{
/*    int64_t millisecs = UTS % 1000;
    UTS /= 1000;
    char* timebuf = ctime(&UTS);
    printf("%s %d\n", timebuf, millisecs);
*/
    printf("myframecallback is being called==================== %d %d %d %lld\n", 
        length, payloadtype, PTS, UTS);

   fs.write(data, length);
    
  return 0;
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
    int ret = mdemux_dmi_alloc((char*)serverIP.c_str(), 8905, (char*)"admin", (char*)"admin", (char*)"pullstream", NULL, &handle);
    if( !handle )
    {
        printf("mdemux_dmi_alloc error \n");
        return -1;
    }

    mdemux_dmi_setframecallback(handle, MyFrameCallback);

    ret = mdemux_dmi_pullstream(handle, (char*)"live", streamid);
    if( ret < 0 )
    {
        printf("mdemux_dmi_pullstream error \n");
        return -1;
    }

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
</code></pre>