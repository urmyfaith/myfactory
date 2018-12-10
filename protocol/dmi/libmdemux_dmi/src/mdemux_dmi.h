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
	CODEC_INVALID = 0,
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
MResult mdemux_dmi_alloc(void **handle, char *server_ip, int server_port, const char* user_name, const char* password, const char* user_agent, void* user_data);

/**
* set frame data callback
*
* return: <0-error, =0 success
*
* @param handle: instance handle
* @param fcallback: frame data callback function
**/
MResult mdemux_dmi_setframecallback(void* handle, MCodecDemuxFinished fcallback);

/**
* pull video stream
*
* return: 0-success, <0-error
*
* @param handle: instance handle
* @param streamtype: live,vod,proxy
* @param streamid: stream id
**/
MResult mdemux_dmi_pullstream(void* handle, char* stream_type, char* stream_id);

/**
* get stream payload type
*
* return: payload type
*
* @param handle: instance handle
**/
DMIPAYLOADTYPE mdemux_dmi_payloadtype(void* handle);

/**
* dmi event loop
*
* return: 0-success, <0-error
*
* @param handle: instance handle
**/
MResult mdemux_dmi_eventloop(void* handle);

/**
* set video speed
*
* return: 0-success, <0-error
*
* @param handle: instance handle
* @param speed: 1/8,1/4,1/2,1,2,4,8
**/
MResult mdemux_dmi_speedx(void* handle, float speed);

/**
* set video play duration
*
* return: 0-success, <0-error
*
* @param handle: instance handle
* @param duration: play starttime
**/
MResult mdemux_dmi_seek(void* handle, char* duration);

/**
* report infomation that user defined
*
* return: 0-success, <0-error
*
* @param handle: instance handle
* @param infomation: infomation as string
**/
MResult mdemux_dmi_report(void* handle, char* infomation);

/**
* destroy a instance
*
* return: 0-success, <0-error
*
* @param handle: instance handle
**/
MResult mdemux_dmi_free(void *handle);

#ifdef __cplusplus
}
#endif

#endif
