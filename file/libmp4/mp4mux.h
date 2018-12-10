/********************************************************************  
filename:   MP4Encoder.h 
created:    2013-04-16 
author:     firehood  
purpose:    MP4编码器，基于开源库mp4v2实现（https://code.google.com/p/mp4v2/）。 
*********************************************************************/  
#pragma once  
#include <mp4v2/mp4v2.h>
  
// NALU单元  
typedef struct _MP4ENC_NaluUnit  
{  
    int type;  
    int size;  
    unsigned char *data;  
}MP4ENC_NaluUnit;  
  
typedef struct _MP4ENC_Metadata  
{  
    // video, must be h264 type  
    unsigned int    nSpsLen;  
    unsigned char   Sps[1024];  
    unsigned int    nPpsLen;  
    unsigned char   Pps[1024];  
  
} MP4ENC_Metadata,*LPMP4ENC_Metadata;  
  
class MP4Encoder  
{  
public:  
    MP4Encoder(void);  
    ~MP4Encoder(void);  
public:  
    // open or creat a mp4 file.  
    int CreateMP4File(const char *fileName,int timeScale = 90000);  

    // wirte 264 metadata in mp4 file.  
    int Write264DecoderConfiguration(const char* sps, int spslen, const char* pps, int ppslen,
                int width, int height, int timescale, int frameRate);

    // wirte 264 data, data can contain  multiple frame.  
    int WriteH264Data(const char* pData,int size);   
    
    // wirte 264 metadata in mp4 file.  
    int WriteAACDecoderConfiguration(int samplerate, int profilelevel);
    int WritePCMDecoderConfiguration(int samplerate, int profilelevel);

    // wirte 264 data, data can contain  multiple frame.  
    int WriteAACData(const char* pData,int size);   

    // close mp4 file.  
    void CloseMP4File();  

    int Test(void);
private:  

private:  
    MP4FileHandle hMp4File;
    MP4TrackId m_videoId;  
    MP4TrackId m_audioId;  
    
    int m_nWidth;  
    int m_nHeight;  
    int m_nFrameRate;  
    int m_nTimeScale;  
};   
