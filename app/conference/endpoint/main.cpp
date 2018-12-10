#include <stdio.h>

#include "demuxer.h"
#include "muxer.h"

#include "../../../transport/udp/udpclientex.h"
#include "../../../util/thread.h"
#include "../../../protocol/librtp/rtpdemux_h264.h"
#include "../../../util/filewritter.h"

void *filewritterhandle = NULL;
void *rtpdemux_h264 = NULL;
void *threadhandle = NULL;
char* outputfilepath = NULL;

int on_rtpdemux_h264frame_callback(const char* frame, int framelength, uint32_t timestamp, uint32_t ssrc)
{
    printf("on_rtpdemux_h264frame_callback length=%u, timestamp=%u\n", framelength, timestamp);
/*
    int frametype = frame[4]&0x1f;
    printf("frameinfo:%u %u %u %u %u frametype:%d framelength:%d \n", 
          frame[0], frame[1], frame[2], frame[3],frame[4], 
          frametype, framelength);
*/
    filewritter_write(filewritterhandle, (char*)frame, framelength);

    return 0;
}

int recvfile(void* usrdata)
{
    void* udphandle = usrdata;
    rtpdemux_h264 = rtpdemux_h264_alloc(on_rtpdemux_h264frame_callback);
//    filewritterhandle = filewritter_alloc("D:/1.264", 1);
    filewritterhandle = filewritter_alloc(outputfilepath, 1);

    char buffer[2000] = {0};
    const char* remoteip = NULL;
    int remoteport = 0;
    while( thread_testcancel(threadhandle) )
    {
        if( udp_clientex_selectread(udphandle) )
        {
            int ret = udp_clientex_read(udphandle, buffer, sizeof(buffer), &remoteip, &remoteport);
            printf("udp_clientex_read:%u \n", ret);
            ret = rtpdemux_h264_setpacket(rtpdemux_h264, buffer, ret);
        }
    }
}

int sendfile(const char* filepath)
{
    const char* remoteip = "192.168.0.100";//"127.0.0.1";
    int remoteport = 11011;
    void* udphandle = udp_clientex_new(-1, 0);
//    void* udphandle = udp_client_new("127.0.0.1", 11011, -1, 0);
    if( !udphandle )
    {
      printf("udp_client_new error \n");
      return NULL;
    }

    int ret = thread_alloc(&threadhandle, recvfile, udphandle);
    ////////////////////////////////////////////////
    void *muxerhandle = muxer_alloc("264");
    void *framerhandle = demuxer_alloc(filepath);

    while( 1 )
    {
        const char *h264frame = NULL;
        int framelength = -1;
        int ret = demuxer_getframe(framerhandle, &h264frame, &framelength);
        if( ret < 0 )
        {
            printf("framer_getframe error\n");
            break;
        }

        muxer_setframe(muxerhandle, h264frame, framelength);

        const char* rtp_buffer = NULL;
        int rtp_packet_length, last_rtp_packet_length, rtp_packet_count;
        muxer_getpacket(muxerhandle, &rtp_buffer, &rtp_packet_length, &last_rtp_packet_length, &rtp_packet_count);

        printf("rtp_packet_length=%d, last_rtp_packet_length=%d,rtp_packet_count=%d,framelength=%d\n", 
            rtp_packet_length,last_rtp_packet_length,rtp_packet_count,framelength);
        for( int i = 0;i < rtp_packet_count;i++ )
        {
            if( i == rtp_packet_count - 1)
            {
                ret = udp_clientex_write(udphandle, rtp_buffer, last_rtp_packet_length, remoteip, remoteport);
//                ret = udp_client_write(udphandle, rtp_buffer, last_rtp_packet_length);
            }
            else
            {
                ret = udp_clientex_write(udphandle, rtp_buffer, rtp_packet_length, remoteip, remoteport);
//                ret = udp_client_write(udphandle, rtp_buffer, rtp_packet_length);
            }

            if( ret < 0 )
            {
              printf("udp_client_write error\n");
              break;
            }
            rtp_buffer += rtp_packet_length;
        }

        if( ret < 0 )
            break;
        usleep(1000 * 40);
//        usleep(1000 * 1000);
    }
  
    udp_clientex_free(udphandle);
    muxer_free(muxerhandle);
    demuxer_free(framerhandle);
    printf("udpmediaproc exit \n");

	return 0;
}

int main(int argc, char *argv[])
{
    printf("====argc==%d,%s\n", argc, argv[argc-1]);
    outputfilepath = argv[argc-1];
	sendfile("D:/test1.264");
	
	return 0;
}