/********************************************************************  
filename:   MP4Encoder.cpp 
created:    2013-04-16 
author:     firehood  
purpose:    MP4±àÂëÆ÷£¬»ùÓÚ¿ªÔ´¿âmp4v2ÊµÏÖ£¨https://code.google.com/p/mp4v2/£©¡£ 
*********************************************************************/  
#include <string.h>  
#include <iostream>

#include "mp4mux.h"  

#define BUFFER_SIZE  (1024*1024)  
  
MP4Encoder::MP4Encoder(void):  
m_videoId(0),  
m_nWidth(0),  
m_nHeight(0),  
m_nTimeScale(90000),  
m_nFrameRate(25)  
{  
}  
  
MP4Encoder::~MP4Encoder(void)  
{  
}  
  
int MP4Encoder::Test(void)  
{  
    MP4TrackId video;  
    MP4TrackId audio;  
    MP4FileHandle fileHandle;  
    unsigned char sps_pps_640[17] = {0x67, 0x42, 0x40, 0x1F, 0x96 ,0x54, 0x05, 0x01, 0xED, 0x00, 0xF3, 0x9E, 0xA0, 0x68, 0xCE, 0x38, 0x80}; //存储sps和pps  
    int video_width = 640;  
    int video_height = 480;  

    fileHandle = MP4Create("test.mp4", 0);  
    if(fileHandle == MP4_INVALID_FILE_HANDLE)  
    {  
        return -1;    
    }  
    video_width = 640;  
    video_height = 480;  
    //设置mp4文件的时间单位  
    MP4SetTimeScale(fileHandle, 90000);  
    //创建视频track //根据ISO/IEC 14496-10 可知sps的第二个，第三个，第四个字节分别是 AVCProfileIndication,profile_compat,AVCLevelIndication     其中90000/20  中的20>是fps  
    video = MP4AddH264VideoTrack(fileHandle, 90000, 90000/20, 
        video_width, video_height, sps_pps_640[1], 
        sps_pps_640[2], sps_pps_640[3], 3);  
    if(video == MP4_INVALID_TRACK_ID)  
    {  
        MP4Close(fileHandle, 0);  
        return -1;    
    }  
    MP4AddH264SequenceParameterSet(fileHandle, video, &(sps_pps_640[0]), 13);  
    MP4AddH264PictureParameterSet(fileHandle, video, sps_pps_640+13, 4);  
    MP4SetVideoProfileLevel(fileHandle, 0x7F);  

    audio = MP4AddAudioTrack(fileHandle, 16000, 1024, MP4_MPEG2_AAC_LC_AUDIO_TYPE);  
    if(audio == MP4_INVALID_TRACK_ID)  
    {  
        MP4Close(fileHandle, 0);  
        return -1;    
    }  
    //设置sps和pps  
    MP4SetAudioProfileLevel(fileHandle, 0x02);  
//    MP4SetTrackESConfiguration(fileHandle, audio, &ubuffer[0], 2);  

    MP4Close(fileHandle, 0);  

    return 0;
}  

int MP4Encoder::CreateMP4File(const char *pFileName,int timeScale/* = 90000*/)  
{  
    if(pFileName == NULL)  
    {  
        return -1;  
    }  
    // create mp4 file  
    hMp4File = MP4Create(pFileName);  
    if (hMp4File == MP4_INVALID_FILE_HANDLE)  
    {  
        printf("ERROR:Open file fialed.\n");  
        return -1;  
    }  
    m_nTimeScale = timeScale;  
    bool ret = MP4SetTimeScale(hMp4File, m_nTimeScale);  
    if( !ret )
    {
        printf("MP4SetTimeScale error.\n");  
        return -1;  
    }

    return 0;  
}  
  
int MP4Encoder::Write264DecoderConfiguration(const char* sps, int spslen, const char* pps, int ppslen,
        int width, int height, int timescale, int frameRate)
{  
    m_nWidth = width;
    m_nHeight = height;
    m_nFrameRate = frameRate;  
    m_nTimeScale = timescale;
    
    m_videoId = MP4AddH264VideoTrack  
        (hMp4File,   
        timescale,   
        timescale / frameRate,   
        width, // width  
        height,// height  
        sps[1], // sps[1] AVCProfileIndication  
        sps[2], // sps[2] profile_compat  
        sps[3], // sps[3] AVCLevelIndication  
        3);           // 4 bytes length before each NAL unit  
    if (m_videoId == MP4_INVALID_TRACK_ID)  
    {  
        printf("add video track failed.\n");  
        return -1;  
    }  
    MP4SetVideoProfileLevel(hMp4File, sps[3]); //  Simple Profile @ Level 3  
  
    // write sps  
    MP4AddH264SequenceParameterSet(hMp4File,m_videoId, (uint8_t*)sps, spslen);  
  
    // write pps  
    MP4AddH264PictureParameterSet(hMp4File,m_videoId, (uint8_t*)pps, ppslen);  
  
    return 0;  
}  
  
int MP4Encoder::WriteH264Data(const char* pData,int size)
{
    if(pData == NULL)  
    {  
        return -1;  
    }  
    if (hMp4File == NULL ||  m_videoId == MP4_INVALID_TRACK_ID)  
    {  
        printf("file not inited.\n");  
        return -1;  
    }            

    MP4ENC_NaluUnit nalu;  
    nalu.data = (unsigned char*)pData;
    nalu.size = size;
    nalu.type = pData[0]&0x1F;
//    printf("frame len=%d\n", nalu.size);

    if(nalu.type == 0x07) // sps  
    {  
        std::cout<<m_nTimeScale<<" "<<
            m_nTimeScale/m_nFrameRate<<" "<<
            m_nWidth<<" "<<m_nHeight<<" "<<
            int(nalu.data[0])<<" "<<int(nalu.data[1])<<" "<<int(nalu.data[2])<<" "<<int(nalu.data[3])<<" "<<
            m_videoId<<" "<<
            std::endl;

        MP4SetVideoProfileLevel(hMp4File, nalu.data[3]); //  Simple Profile @ Level 3  

        MP4AddH264SequenceParameterSet(hMp4File,m_videoId,nalu.data,nalu.size);  
    }  
    else if(nalu.type == 0x08) // pps  
    {  
        MP4AddH264PictureParameterSet(hMp4File,m_videoId,nalu.data,nalu.size);  
    }  
    else  
    {  
        int datalen = nalu.size+4;  
        unsigned char *data = new unsigned char[datalen];  
        // MP4 NaluÇ°ËÄ¸ö×Ö½Ú±íÊ¾Nalu³¤¶È  
        data[0] = nalu.size>>24;  
        data[1] = nalu.size>>16;  
        data[2] = nalu.size>>8;  
        data[3] = nalu.size&0xff;  
        memcpy(data+4,nalu.data,nalu.size);  

        int iskeyframe = (nalu.type == 0x05);
//        if(!MP4WriteSample(hMp4File, m_videoId, data, datalen,3600, 0, iskeyframe))  
        if(!MP4WriteSample(hMp4File, m_videoId, data, datalen,MP4_INVALID_DURATION, 0, iskeyframe))  
        {  
            printf("MP4WriteSample error.\n");  
            return 0;  
        }  
        delete[] data;  
    }  
          
    return 0;  
}
  
int MP4Encoder::WriteAACDecoderConfiguration(int samplerate, int profilelevel)
{
    m_audioId = MP4AddAudioTrack(hMp4File, samplerate, 1024, MP4_MPEG2_AAC_LC_AUDIO_TYPE);  
    //设置sps和pps  
    MP4SetAudioProfileLevel(hMp4File, profilelevel);  
//    MP4SetTrackESConfiguration(hMp4File, m_audioId, &ubuffer[0], 2);  

    return 0;
}

int MP4Encoder::WritePCMDecoderConfiguration(int samplerate, int profilelevel)
{
    m_audioId = MP4AddAudioTrack(hMp4File, samplerate, 1024, MP4_PCM16_LITTLE_ENDIAN_AUDIO_TYPE);  
    if(m_audioId == MP4_INVALID_TRACK_ID)  
    {  
        printf("MP4AddAudioTrack error\n");
        return -1;    
    }      
    //设置sps和pps  
    MP4SetAudioProfileLevel(hMp4File, profilelevel);  
//    MP4SetTrackESConfiguration(hMp4File, m_audioId, &ubuffer[0], 2);  

    return 0;
}

int MP4Encoder::WriteAACData(const char* pData,int size)
{
    if(m_audioId == MP4_INVALID_TRACK_ID)  
    {  
        printf("MP4AddAudioTrack error\n");
        return -1;    
    }      

    MP4WriteSample(hMp4File, m_audioId, (unsigned char*)pData, size , MP4_INVALID_DURATION, 0, 1);

    return 0;  
}

void MP4Encoder::CloseMP4File()  
{  
    if(hMp4File)  
    {  
        MP4Close(hMp4File);  
        hMp4File = NULL;  
    }  
}
