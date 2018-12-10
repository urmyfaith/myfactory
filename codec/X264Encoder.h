/*
 * @file x264_encoder.c
 * @author Akagi201
 * @date 2014/12/31
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern "C"
{
	#include "x264.h"
};

class x264Encoder
{
public:
	x264Encoder();
	~x264Encoder();

	bool InitX264Encoder(unsigned short usWidth,unsigned short usHeight,int nFrameRate,int nBitRate ); 
	bool X264Encode(unsigned char* pInFrame, x264_nal_t **nals, int& nnal); 
	void ReleaseConnection(); 

	//frameType:6-SEI;7-SPS;8-PPS
	bool GetHeaderFrame(unsigned char* pOutFrame,int& nOutLen, int frameType);
private:
	int encode_nals(unsigned char *buf, x264_nal_t *nals, int nnal);
	void _SetFastParam(x264_param_t *param);

private: 
	x264_t *h; 

	unsigned short  m_usWidth; 
	unsigned short  m_usHeight; 

	x264_picture_t  m_pic_in; 
	x264_picture_t  m_pic_out; 
	unsigned m_pts;
};
