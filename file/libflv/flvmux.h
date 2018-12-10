#ifndef __FLVMUX_H__
#define __FLVMUX_H__

#include <fstream>

namespace Cnvt
{
	class u4
	{
	public:
		u4(unsigned int i) { _u[0] = i >> 24; _u[1] = (i >> 16) & 0xff; _u[2] = (i >> 8) & 0xff; _u[3] = i & 0xff; }

	public:
		unsigned char _u[4];
	};
	class u3
	{
	public:
		u3(unsigned int i) { _u[0] = i >> 16; _u[1] = (i >> 8) & 0xff; _u[2] = i & 0xff; }

	public:
		unsigned char _u[3];
	};
	class u2
	{
	public:
		u2(unsigned int i) { _u[0] = i >> 8; _u[1] = i & 0xff; }

	public:
		unsigned char _u[2];
	};

#pragma pack(push, 1)
	typedef struct FLVHeader  
	{  
	  unsigned char First;// "F"  
	  unsigned char Second;//"L"  
	  unsigned char Last;//"V"  
	  unsigned char Version; //0x01  
	  unsigned char Flags;//"ǰ��λ������Ϊ0������λ��ʾ�Ƿ������ƵTag������λ����Ϊ0���ڰ�λ��ʾ�Ƿ�����ƵTag"  
	  unsigned char HeaderLenth[4];//header ���ȣ�һ��9bytes  
	}FLVHeader;  

	typedef struct TagHeader  
	{  
	  unsigned char TagType;//tag���ͣ���ƵΪ(0X08),��Ƶ(0X09),script data(0x12),����ֵ����  
	  unsigned char DataSize[3];//3�ֽڱ�ʾTagdata�Ĵ�С  
	  unsigned char Ts[3]      ;     //3�ֽڣ�ΪTag��ʱ�����    
	  unsigned char TsEx     ;//ʱ�������չλ����24λ������ʱ���˰�λ��Ϊʱ������λ����ʱ�����Ϊ32λ  
	  unsigned char StreamID[3];//һֱΪ0��  
	}TagHeader;  

	typedef struct ScriptTagData  
	{  
	  unsigned char MetaDataType;//0x02  
	  unsigned char StringLenth[2];//һ��λ10����0x000A��  
	  unsigned char MetaString[10];//ֵΪonMetaDat  
	  unsigned char InfoDataType;//0x08��ʾ���飬Ҳ���ǵڶ���AMF��  
	  unsigned char EnumNum[4];//4bytes�ж��ٸ�Ԫ��//18bytes  
	  //1  
	  unsigned char DurationLenth[2];//2bytes,duration�ĳ���  
	  unsigned char DurationName[8];  
	  unsigned char DurationType;  
	  unsigned char DurationData[8];  
	  //2  
	  unsigned char WidthLenth[2];//  
	  unsigned char WidthName[5];  
	  unsigned char WidthType;  
	  unsigned char WidthData[8];  
	  //3  
	  unsigned char HeightLenth[2];  
	  unsigned char HeightName[6];  
	  unsigned char HeightType;  
	  unsigned char HeightData[8];  
	  //4  
	  unsigned char FrameRateLenth[2];  
	  unsigned char FrameRateName[9];  
	  unsigned char FrameRateType;  
	  unsigned char FrameRateData[8];  
	  //5  
	  unsigned char FileSizeLenth[2];  
	  unsigned char FileSizeName[8];  
	  unsigned char FileSizeType;  
	  unsigned char FileSizeData[8];  
    
	  unsigned char End[3];//0x000009  
	}ScriptTagData;  

	typedef struct VideoTagData
	{  
	  unsigned char FrameCodecType;//��4λ,��ʾ֡���ͣ���4λ,��ʾ��������  
	  unsigned char AVPacketType;//0 = AVC sequence header.....1 = AVC NALU.....2 = AVC end of sequence (lower level NALU   
	  unsigned char CompositionTime[3];//Ĭ��Ϊ0  
	  unsigned char DataLenth[4];//data����+ǰ���9�ֽڣ�ע�⣬ScriptTag��ĵ�һ��Tagû�д��ֶ�  
	  unsigned char *Data;  
	}VideoTagData;  

	typedef struct AudioTagData
	{  
	  unsigned char SoundInfo;//��4λ,��ʾ֡���ͣ���4λ,��ʾ��������  
	  unsigned char *Data;  //AACPacketType
	}AudioTagData;  
#pragma pack(pop)

	class CConverter
	{
	public:
		CConverter();
		virtual ~CConverter();

		int Open(std::string strFlvFile, int bHaveAudio=0, int bHaveVideo=1);
		int Close();

		int WriteH264Header(const unsigned char* sps, int spslen, const unsigned char* pps, int ppslen);
		int WriteH264Frame(const char *pNalu, int nNaluSize, unsigned int nTimeStamp);

		int WriteAACHeader(uint32_t profile, uint32_t sampleRateIndex, uint32_t channelConfig);
		int WriteAACFrame(const char *pFrame, int nFrameSize, unsigned int nTimeStamp);
	private:
		void _WriteFlvHeader();
		void _WriteMetaData();

		void WriteH264EndofSeq();

		void Write(unsigned char u) { _fileOut.write((char *)&u, 1); }
		void Write(u4 u) { _fileOut.write((char *)u._u, 4); }
		void Write(u3 u) { _fileOut.write((char *)u._u, 3); }
		void Write(u2 u) { _fileOut.write((char *)u._u, 2); }

	private:
		FLVHeader m_flvHeader;

		unsigned char *_pSPS, *_pPPS;
		int _nSPSSize, _nPPSSize;
		int _bWriteAVCSeqHeader;
		int _nPrevTagSize;
		int _nStreamID;
		int _nVideoTimeStamp;

		unsigned char *_pAudioSpecificConfig;
		int _nAudioConfigSize;
		int _aacProfile;
		int _sampleRateIndex;
		int _channelConfig;
		int _bWriteAACSeqHeader;

	private:
		std::fstream _fileOut;

	private:
		int _bHaveAudio, _bHaveVideo;

	};

}

#endif // CONVERTER_H
