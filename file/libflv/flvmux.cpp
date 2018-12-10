#include <string.h>
#include <stdio.h>
#include <iostream>

#include "flvmux.h"

namespace Cnvt
{
	CConverter::CConverter()
	{
		_pSPS = NULL;
		_pPPS = NULL;
		_nSPSSize = 0;
		_nPPSSize = 0;
		_bWriteAVCSeqHeader = 0;
		_nPrevTagSize = 0;
		_nStreamID = 0;

		_pAudioSpecificConfig = NULL;
		_nAudioConfigSize = 0;
		_aacProfile = 0;
		_sampleRateIndex = 0;
		_channelConfig = 0;
		_bWriteAACSeqHeader = 0;
	}

	CConverter::~CConverter()
	{

	}

	int CConverter::Open(std::string strFlvFile, int bHaveAudio, int bHaveVideo)
	{
		_fileOut.open(strFlvFile, std::ios_base::out | std::ios_base::binary);
		if (!_fileOut)
			return 0;

		_bHaveAudio = bHaveAudio;
		_bHaveVideo = bHaveVideo;

		_WriteFlvHeader();

		return 1;
	}

	int CConverter::Close()
	{
		if (_pSPS != NULL)
			delete _pSPS;
		if (_pPPS != NULL)
			delete _pPPS;
		
		if (_bHaveVideo!=0)
			WriteH264EndofSeq();

		_fileOut.close();

		return 1;
	}
	void CConverter::_WriteFlvHeader()
	{
		m_flvHeader.First = 'F';
		m_flvHeader.Second = 'L';
		m_flvHeader.Last = 'V';
		m_flvHeader.Version = 0x01;
		m_flvHeader.Flags = 0;
		if ( _bHaveVideo != 0 )
			m_flvHeader.Flags |= 0x01;
		if ( _bHaveAudio != 0 )
			m_flvHeader.Flags |= 0x04;

		unsigned int size = 9;
		u4 size_u4(size);
		memcpy(&m_flvHeader.HeaderLenth, size_u4._u, sizeof(unsigned int));

		_fileOut.write((char *)&m_flvHeader, sizeof(m_flvHeader) );

		u4 prev_u4(0);
		_fileOut.write((char *)prev_u4._u, 4);
	}
	void CConverter::_WriteMetaData()
	{
		TagHeader scriptTagHeader;
		memset( &scriptTagHeader, 0 ,sizeof(scriptTagHeader) );		
		scriptTagHeader.TagType = 0x12;
		Cnvt::u3 _u3(sizeof(ScriptTagData));
		memcpy( scriptTagHeader.DataSize, _u3._u, sizeof(_u3));
		_fileOut.write((char *)&scriptTagHeader, sizeof(scriptTagHeader) );

		ScriptTagData scriptTagData;
		memset( &scriptTagData, 0 ,sizeof(scriptTagData) );		
//		scriptTagData.
	}

	int CConverter::WriteH264Header(const unsigned char* sps, int spslen, const unsigned char* pps, int ppslen)
	{
		char cTagType = 0x09;
		_fileOut.write(&cTagType, 1);
		int nDataSize = 1 + 1 + 3 + 6 + 2 + spslen + 1 + 2 + ppslen;

		printf("WriteH264Header2 %d %d %d %d %d\n", spslen, ppslen, sps[1], sps[2], sps[3]);

		u3 datasize_u3(nDataSize);
		_fileOut.write((char *)datasize_u3._u, 3);

		u3 tt_u4(0);
		_fileOut.write((char *)tt_u4._u, 4);

		u3 sid_u3(_nStreamID);
		_fileOut.write((char *)sid_u3._u, 3);

		unsigned char cVideoParam = 0x17;
		_fileOut.write((char *)&cVideoParam, 1);
		unsigned char cAVCPacketType = 0; /* seq header */
		_fileOut.write((char *)&cAVCPacketType, 1);

		u3 CompositionTime_u3(0);
		_fileOut.write((char *)CompositionTime_u3._u, 3);

		Write(1);// configurationVersion
		Write((unsigned char)sps[1]); // AVCProfileIndication
		Write((unsigned char)sps[2]);// profile_compatibility
		Write((unsigned char)sps[3]); // AVCLevelIndication
		// 6 bits reserved (111111) + 2 bits nal size length - 1
		// (Reserved << 2) | Nal_Size_length = (0x3F << 2) | 0x03 = 0xFF
		Write((unsigned char)0xff);
		// 3 bits reserved (111) + 5 bits number of sps (00001)
		// (Reserved << 5) | Number_of_SPS = (0x07 << 5) | 0x01 = 0xe1
		Write((unsigned char)0xE1);

		u2 spssize_u2(spslen);
		_fileOut.write((char *)spssize_u2._u, 2);
		_fileOut.write((char *)sps, spslen);

		//pps
		Write((unsigned char)0x01);
		u2 ppssize_u2(ppslen);
		_fileOut.write((char *)ppssize_u2._u, 2);
		_fileOut.write((char *)pps, ppslen);

		_nPrevTagSize = 11 + nDataSize;
		u4 prev_u4(_nPrevTagSize);
		_fileOut.write((char *)prev_u4._u, 4);

		return 0;
	}

	int CConverter::WriteH264Frame(const char *pNalu, int nNaluSize, unsigned int nTimeStamp)
	{	
		int nNaluType = pNalu[0] & 0x1f;
		if (nNaluType == 6 || nNaluType == 7 || nNaluType == 8)
			return 0;

		Write(0x09);
		int nDataSize;
		nDataSize = 1 + 1 + 3 + 4 + (nNaluSize);
		u3 datasize_u3(nDataSize);
		Write(datasize_u3);
		u3 tt_u3(nTimeStamp);
		Write(tt_u3);
		Write((unsigned char)(nTimeStamp >> 24));

		u3 sid(_nStreamID);
		Write(sid);

		if (nNaluType == 5)
			Write(0x17);
		else
			Write(0x27);
		Write((unsigned char)(1));
		u3 com_time_u3(0);
		Write(com_time_u3);

		u4 nalusize_u4(nNaluSize);
		Write(nalusize_u4);

		_fileOut.write((char *)(pNalu), nNaluSize);

		_nPrevTagSize = 11 + nDataSize;
		u4 prev_u4(_nPrevTagSize);
		Write(prev_u4);

		return 0;
	}

	void CConverter::WriteH264EndofSeq()
	{
		Write(0x09);
		int nDataSize;
		nDataSize = 1 + 1 + 3;
		u3 datasize_u3(nDataSize);
		Write(datasize_u3);
		u3 tt_u3(_nVideoTimeStamp);
		Write(tt_u3);
		Write((unsigned char)(_nVideoTimeStamp >> 24));

		u3 sid(_nStreamID);
		Write(sid);

		Write(0x27);
		Write(0x02);

		u3 com_time_u3(0);
		Write(com_time_u3);
	}

	int CConverter::WriteAACHeader(uint32_t profile, uint32_t sampleRateIndex, uint32_t channelCount)
	{
		unsigned char pAudioSpecificConfig[2] = {0};
		pAudioSpecificConfig[0] = (profile << 3) + (sampleRateIndex>>1);
		pAudioSpecificConfig[1] = ((sampleRateIndex&0x01)<<7) + (channelCount<<3);
		printf("_aacProfile %d %d %d %d %d \n", profile, sampleRateIndex, 
			channelCount, pAudioSpecificConfig[0], pAudioSpecificConfig[1]);

		char cTagType = 0x08;
		_fileOut.write(&cTagType, 1);
		int nDataSize = 1 + 1 + 2;

		u3 datasize_u3(nDataSize);
		_fileOut.write((char *)datasize_u3._u, 3);

		unsigned int nTimeStamp = 0;
		u3 tt_u3(nTimeStamp);
		_fileOut.write((char *)tt_u3._u, 3);

		unsigned char cTTex = nTimeStamp >> 24;
		_fileOut.write((char *)&cTTex, 1);

		u3 sid_u3(_nStreamID);
		_fileOut.write((char *)sid_u3._u, 3);

		unsigned char cAudioParam = 0xAF;
		_fileOut.write((char *)&cAudioParam, 1);
		unsigned char cAACPacketType = 0; /* seq header */
		_fileOut.write((char *)&cAACPacketType, 1);

		_fileOut.write((char *)pAudioSpecificConfig, 2);

		_nPrevTagSize = 11 + nDataSize;
		u4 prev_u4(_nPrevTagSize);
		_fileOut.write((char *)prev_u4._u, 4);

		return 0;
	}

	int CConverter::WriteAACFrame(const char *pFrame, int nFrameSize, unsigned int nTimeStamp)
	{
		Write(0x08);
		int nDataSize;
		nDataSize = 1 + 1 + (nFrameSize);
		u3 datasize_u3(nDataSize);
		Write(datasize_u3);
		u3 tt_u3(nTimeStamp);
		Write(tt_u3);
		Write((unsigned char)(nTimeStamp >> 24));

		u3 sid(_nStreamID);
		Write(sid);

		unsigned char cAudioParam = 0xAF;
		_fileOut.write((char *)&cAudioParam, 1);
		unsigned char cAACPacketType = 1; /* AAC raw data */
		_fileOut.write((char *)&cAACPacketType, 1);

		_fileOut.write((char *)pFrame, nFrameSize);

		_nPrevTagSize = 11 + nDataSize;
		u4 prev_u4(_nPrevTagSize);
		Write(prev_u4);

		return 0;
	}
}
