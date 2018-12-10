#ifndef __MDEMUX_DMI_H__
#define __MDEMUX_DMI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>  
#include <stdlib.h>  
#include <stdint.h>

#include "mcodec.h"

typedef enum 
{
	CODEC_H264 = 1,
	CODEC_H265 = 2,
	CODEC_VP8  = 3,
	CODEC_VP9  = 4,

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

typedef struct
{
	char *vps;
	int vps_length;
	char *sps;
	int sps_length;
	char *pps;
	int pps_length;
	int width;
	int height;
}VideoMediaInfo;

typedef struct
{
	uint32_t sample_rate;
	int bits_per_sample;
	int channels;
}AudioMediaInfo;

/**
* alloc a new instance
*
* return: <0-error, =0 success
*
* @param handle: instance handle
* @param server_ip: server ip
* @param server_port: server port
* @param user_name: server username
* @param password: server password
* @param user_agent: user defined agent name
* @param user_data: callback usrdata
**/
MResult mmux_dmi_alloc(void **handle, char *server_ip, int server_port, const char* user_name, const char* password, const char* user_agent, void* user_data);

/**
* play speed callback
*
* return: <0-error, =0 success
*
* @param handle: instance handle
* @param callback: play speed callback function
**/
MResult mmux_dmi_setspeedxcallback(void* handle, MCodecMuxSpeedXCallback callback);

/**
* play seek callback
*
* return: <0-error, =0 success
*
* @param handle: instance handle
* @param callback: play seek callback function
**/
MResult mmux_dmi_setseekcallback(void* handle, MCodecMuxSeekCallback callback);

/**
* player open callback
*
* return: <0-error, =0 success
*
* @param handle: instance handle
* @param callback: player open callback function
**/
MResult mmux_dmi_setplayeropencallback(void* handle, MCodecMuxPlayerOpenCallback callback);

/**
* player close callback
*
* return: <0-error, =0 success
*
* @param handle: instance handle
* @param callback: player close callback function
**/
MResult mmux_dmi_setplayerclosecallback(void* handle, MCodecMuxPlayerCloseCallback callback);

/**
* set video media infomation
*
* return: 0-success, <0-error
*
* @param handle: instance handle
* @param info: video media infomation
**/
MResult mmux_dmi_setvideomediainfo(void* handle, VideoMediaInfo *info);

/**
* set audio media infomation
*
* return: 0-success, <0-error
*
* @param handle: instance handle
* @param info: audio media infomation
**/
MResult mmux_dmi_setaudiomediainfo(void* handle, AudioMediaInfo *info);

/**
* push video stream
*
* return: 0-success, <0-error
*
* @param handle: instance handle
* @param streamtype: live,vod,proxy
* @param streamid: stream id
* @param payloadtype: payload type
**/
MResult mmux_dmi_pushstream(void* handle, char* stream_type, char* stream_id, DMIPAYLOADTYPE payloadtype);

/**
* dmi event loop
*
* return: 0-success, <0-error
*
* @param handle: instance handle
**/
MResult mmux_dmi_eventloop(void* handle);

/**
* send frame data
*
* return: 0-success, <0-error
*
* @param handle: instance handle
* @param framedata: framedata
* @param framelength: framelength
**/
MResult mmux_dmi_sendframe(void* handle, char* frame_data, int frame_length, DMIPAYLOADTYPE payload_type, uint32_t timestamp);

/**
* report infomation that user defined
*
* return: 0-success, <0-error
*
* @param handle: instance handle
* @param infomation: infomation as string
**/
MResult mmux_dmi_report(void* handle, const char* infomation);

/**
* destroy a instance
*
* return: 0-success, <0-error
*
* @param handle: instance handle
**/
MResult mmux_dmi_free(void *handle);

#ifdef __cplusplus
}
#endif

#endif
