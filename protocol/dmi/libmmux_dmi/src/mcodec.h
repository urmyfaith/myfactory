/*
 * Copyright (c) 2017 DeepGlint
 *
 * This file is part of DGMF.
 *
 * DGMF is commercial software; you can redistribute it and/or
 * modify it under the terms of the DeepGlint Commercial Software
 * License as published by DeepGlint Inc;
 *
 * tongliu@deepglint.com
 * 2017-07-10
 */

#ifndef MCODEC_H
#define MCODEC_H

#include <stdint.h>

/**
 * Decode finished callback function, it can be automatically called by decoder
 * after decoding finished. You can get the decoded raw image data, width, height
 * pixel format and it's linesize by this function.
 *
 * @param data 4 channel raw data buffer pointer array
 * @param width Raw image resolution width
 * @param height Raw image resolution height
 * @param linesize Raw image each channel pitch linesize:
 *			PIX_FMT_NV12:					linesize[0] = width, linesize[1] = width, linesize[2] = 0, linesize[3] = 0
 *			PIX_FMT_YUV420P:				linesize[0] = width, linesize[1] = width / 2, linesize[2] = width / 2, linesize[3] = 0
 *			PIX_FMT_RGB, PIX_FMT_BGR:		linesize[0] = width * 3, linesize[1] = 0, linesize[2] = 0, linesize[3] = 0
 *			PIX_FMT_RGBA, PIX_FMT_BGRA: 	linesize[0] = width * 4, linesize[1] = 0, linesize[2] = 0, linesize[3] = 0
 * @param pix_fmt Raw image pixel format, supported formats:
 *			mdec_h264_cuda:		PIX_FMT_NV12
 *			mdec_h264_ffmpeg:	PIX_FMT_YUV420P
 * @param user_data Callback user data					
 */
typedef void (*MCodecDecodeFinished) (uint8_t *data[4], int width, int height, int linesize[4], const char *pix_fmt, void *user_data);

/**
 * Encode finished callback function, it can be automatically called by encoder
 * after encoding finished. You can get the encoded packet data and size by 
 * this function.
 *
 * @param data Encoded packet data pointer
 * @param size Encoded packet data size
 * @param user_data Callback user data
 */
typedef void (*MCodecEncodeFinished) (uint8_t *data, int size, void *user_data);

/**
 * Demux finished callback function, it can be automatically called by demuxer
 * after demuxing finished. You can get the demuxed packet data, size and codec 
 * name by this function.
 *
 * @param data Demuxed packet data pointer
 * @param size Demuxed packet data size
 * @param codec_name packet codec type, supported types:
 *		CODEC_H264, CODEC_H265
 * @param user_data Callback user data
 */
typedef void (*MCodecDemuxFinished) (uint8_t *data, int size, const char *codec_name, uint32_t PTS, uint64_t UTS, void *user_data);

/**
* description: play speed callback function
*
* return:
*
* @param streamid: streamid
* @param speed: play speed
* @param user_data: callback user data
**/
typedef void (*MCodecMuxSpeedXCallback)(char* streamid, float speed, void* user_data);

/**
* description: play seek callback function
*
* return:
*
* @param streamid: streamid
* @param duration: seek duration
* @param user_data: callback user data
**/
typedef void (*MCodecMuxSeekCallback)(char* streamid, char* duration, void* user_data);

/**
* description: player open callback function
*
* return:
*
* @param streamid: streamid
* @param sessionid: sessionid
* @param user_data: callback user data
**/
typedef void (*MCodecMuxPlayerOpenCallback)(char* streamid, char* sessionid, void* user_data);

/**
* description: player close callback function
*
* return:
*
* @param streamid: streamid
* @param sessionid: sessionid
* @param user_data: callback user data
**/
typedef void (*MCodecMuxPlayerCloseCallback)(char* streamid, char* sessionid, void* user_data);

/* 
 * Return result struct
 */
typedef struct MResult {
	int status; 			///< status  >0: successed, status <=0: failed
	const char *error; 		///< error: error reason
} MResult;

#endif //MCODEC_H