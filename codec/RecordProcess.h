#ifndef _RECORDPROCESS_H_
#define _RECORDPROCESS_H_

base::Lock lock__;

volatile unsigned audioDataQueueSize = 0;
volatile unsigned videoDataQueueSize = 0;
queueMEDIADATA audioDataQueue;
queueMEDIADATA videoDataQueue;
MEDIA_DATA h264datatemp;
MEDIA_DATA aacdatatemp;

Cnvt::CConverter convObj;
unsigned __stdcall recordProc(void *)
{
	unsigned videoTimeStamp = 0;
	unsigned audioTimeStamp = 0;

	unsigned long tickcount = 0;
	unsigned audioWritted = 0, videoWritted = 0;
	while( 1 )
	{
// 		printf("timestamp:%d-%d\n", h264datatemp.timestamp_, aacdatatemp.timestamp_);
		char strDump[1024] = {0};
		sprintf(strDump, "timestamp:h264 %u aac %u", h264datatemp.timestamp_, aacdatatemp.timestamp_);
		if( h264datatemp.timestamp_ < ((DWORD32)~((DWORD32)0)) )
			EasyLog::Inst()->Log(strDump);

// 		printf("timestamp:%d-%d\n", videoDataQueueSize, audioDataQueueSize);
		if( videoDataQueueSize > 1 )
		{
			if( tickcount == 0 )
				tickcount = GetTickCount();

//			printf(":");
			h264datatemp = videoDataQueue.front();
			videoDataQueueSize -- ;
			videoDataQueue.pop_front();
			if( videoWritted == 0 )
			{
				videoTimeStamp = 0;
				videoWritted = 1;
			}
			else
			{
				videoTimeStamp = GetTickCount() - tickcount;
			}

//			printf("videoTimeStamp=%u \n", videoTimeStamp );

			convObj.ConvertH264((char*)h264datatemp.mediadata, h264datatemp.mediadatalength, videoTimeStamp);
		}

		if( audioDataQueueSize > 1 )
		{
			if( tickcount == 0 )
				tickcount = GetTickCount();

			if( audioWritted == 0 )
			{
				audioTimeStamp = 0;
				audioWritted = 1;
			}
			else
			{
				audioTimeStamp = GetTickCount() - tickcount;
			}
//			printf(".");
			aacdatatemp = audioDataQueue.front();
			audioDataQueueSize -- ;
			audioDataQueue.pop_front();
			convObj.ConvertAAC((char*)aacdatatemp.mediadata, aacdatatemp.mediadatalength, audioTimeStamp);
		}
		
		Sleep(1);
	}

	return 0;
}

MEDIA_DATA h264data;
int push_h264Frame(unsigned char *p_Out_Frame, unsigned n_OutFrame_Size)
{
	h264data.timestamp_ = 0;
	h264data.mediadatalength = n_OutFrame_Size;
	memset(h264data.mediadata, 0, sizeof(h264data.mediadata));
	memcpy( h264data.mediadata, p_Out_Frame, n_OutFrame_Size);
	videoDataQueue.push_back(h264data);
	videoDataQueueSize ++;

	return 0;
}

int _RecordH264(int isFLV = 0)
{
	DSVideoCapture *ds_capture_ = new DSVideoCapture();
	std::map<CString, CString> a_devices = ds_capture_->DShowGraph()->VideoCapDevices();

	if(a_devices.size() == 0 ) ::exit(0);

	// 枚举音频采样设备，选择最后一个
	CString video_device_id;
	for (std::map<CString, CString>::iterator it = a_devices.begin(); it != a_devices.end(); ++it)
	{
		if( it->second.Find(_T("Camera")) >= 0 )
		{
			video_device_id = it->first;
			break;
		}
	}

	unsigned short usWidth = 320;
	unsigned short usHeight = 240;
	unsigned short fps = 15;
	unsigned long bps = 147456000;

	DSVideoFormat video_fmt;
	video_fmt.width = usWidth;
	video_fmt.height = usHeight;
	video_fmt.fps = fps;
	video_fmt.bps = bps;

    ds_capture_->Create(video_device_id, video_fmt, NULL);
    usWidth = ds_capture_->GetVideoGraph()->GetWidth();
	usHeight = ds_capture_->GetVideoGraph()->GetHeight();
	fps = ds_capture_->GetVideoGraph()->GetFrameRate();
	bps = ds_capture_->GetVideoGraph()->GetBitRate();

	x264Encoder x264wrapper;
	bool bRet = x264wrapper.InitX264Encoder(usWidth, usHeight, 10, 1024);

	// 开始采集音频数据
    ds_capture_->Start();

	::Sleep(10);

	unsigned char *p_Out_Frame = new unsigned char[usWidth * usHeight * 3/2]; 
	unsigned char *yuvbuf = new unsigned char[usWidth * usHeight * 3];

	// 编码后存文件
	FILE *f_h264_ = fopen("./video.h264","wb");
	printf("Recording...");
	while(1)
	{
		unsigned tick1 = GetTickCount();
		// 采集音频pcm数据
		int ret = ds_capture_->GetBuffer(h264data);
//		printf("Video buffersize =%u tickcost=%u \n",ds_capture_->BufferSize(), GetTickCount() - tick1 );

		if(ret >= 0)
		{
			unsigned long len = 0;
			tick1 = GetTickCount();
			if( !RGB2YUV420((LPBYTE)h264data.mediadata, usWidth, usHeight, (LPBYTE)yuvbuf, &len ) )
			{
				std::cout<<"RGB2YUV error"<<std::endl;
				return -1;
			}
//			printf("Video buffersize =%u ===tickcost=%u \n",ds_capture_->BufferSize(), GetTickCount() - tick1 );

			x264_nal_t *nals = NULL;
			int nnal;
			tick1 = GetTickCount();
			bool bRet = x264wrapper.X264Encode(yuvbuf, &nals, nnal);
//			if( !bRet )
//				bRet = x264wrapper.X264Encode(NULL, &nals, nnal);

			if( bRet )
			{
//				printf("Video buffersize =%u ========tickcost=%u \n",ds_capture_->BufferSize(), GetTickCount() - tick1 );

				tick1 = GetTickCount();
				if( isFLV == 0 )
				{
					for ( int j = 0; j < nnal; ++j)
					{
						fwrite(nals[j].p_payload, nals[j].i_payload, 1, f_h264_);
					}
				}
				else
				{
					for ( int j = 0; j < nnal; ++j)
					{
						push_h264Frame(nals[j].p_payload, nals[j].i_payload);
					}
				}
				printf("Video buffersize =%u tickcost=%u \n",ds_capture_->BufferSize(), GetTickCount() - tick1 );
			}
		}
		::Sleep(1);
	}

	SAFE_DELETE(p_Out_Frame);
	SAFE_DELETE(yuvbuf);

	fclose(f_h264_);
}

MEDIA_DATA aacdata;
int _RecordAAC(int isFLV = 0)
{
	unsigned fBitsPerSample = 16; //单个采样音频信息位数
	unsigned fNumChannels = 2; //通道数
	unsigned fSamplingFrequency = 44100;//采样率

	DSAudioCapture *ds_capture_ = new DSAudioCapture();
	std::map<CString, CString> a_devices = ds_capture_->DShowGraph()->AudioCapDevices();

	if(a_devices.size() == 0 ) ::exit(0);

	DSAudioFormat audio_fmt;
	audio_fmt.samples_per_sec = fSamplingFrequency;
	audio_fmt.channels = fNumChannels;
	audio_fmt.bits_per_sample = fBitsPerSample;

	// 枚举音频采样设备，选择最后一个
	CString audio_device_id;
	for (std::map<CString, CString>::iterator it = a_devices.begin(); it != a_devices.end(); ++it)
    {
		if( it->second.Find(_T("麦克风")) >= 0 )
		{
			audio_device_id = it->first;
			break;
		}
    }
	ds_capture_->Create(audio_device_id, audio_fmt, NULL);

	fBitsPerSample = ds_capture_->GetAudioGraph()->GetBitsPerSample();
	fNumChannels = ds_capture_->GetAudioGraph()->GetChannels();
	fSamplingFrequency = ds_capture_->GetAudioGraph()->GetSamplesPerSec();

	FAACEncoder *faac_encoder_ = new FAACEncoder(); //新建AAC编码对象
	faac_encoder_->Init(fSamplingFrequency,fNumChannels,fBitsPerSample); //初始化AAC编码器

	// 开始采集音频数据
	ds_capture_->Start();

	::Sleep(10);

	unsigned long max_out_bytes = faac_encoder_->MaxOutBytes();
	unsigned char* outbuf = (unsigned char*)malloc(max_out_bytes);

	// 编码后存文件
	FILE *f_aac_ = fopen("./audio.aac","wb");
	printf("Recording...");
	while(1)
	{
		// 采集音频pcm数据
		int ret = ds_capture_->GetBuffer(aacdata);
//		printf("Audio buffersize =%u \n",ds_capture_->BufferSize() );

		if(ret >= 0)
		{
			unsigned int sample_count = (aacdata.mediadatalength << 3)/fBitsPerSample;
			unsigned int buf_size = 0;
			// 编码AAC
			faac_encoder_->Encode((unsigned char*)aacdata.mediadata, sample_count, (unsigned char*)outbuf, buf_size);	
			if(buf_size > 0)
			{
//				printf("aacdata.timestamp_:%d-%d\n", aacdata.timestamp_, nTimeStamp);
// 				char strDump[1024] = {0};
// 				sprintf(strDump, "aacdata.timestamp_:%d %d %d\n", aacdata.timestamp_, nTimeStamp, aacdata.timestamp_ - nTimeStamp);
//				EasyLog::Inst()->Log(strDump);

				// 存文件或者自定义
				if( isFLV == 0 )
					fwrite(outbuf, 1, buf_size, f_aac_);
				else
				{
					aacdata.timestamp_ = 0;
					aacdata.mediadatalength = buf_size;
					memcpy( aacdata.mediadata, outbuf, buf_size);
					audioDataQueue.push_back(aacdata);
					audioDataQueueSize ++;
				}
			}			
		}
		::Sleep(1);
	}

	free(outbuf);
}

unsigned __stdcall encodeH264Proc(void *)
{
	_RecordH264(1);
//	_RecordH264(0);

	return 0;
}

unsigned __stdcall encodeAACProc(void *)
{
	_RecordAAC(1);

	return 0;
}

int RecordFLV()
{
	convObj.Open("./av.flv", 1, 1);

	_beginthreadex(0, 0, encodeH264Proc, 0, 0, 0);
	_beginthreadex(0, 0, encodeAACProc, 0, 0, 0);
	_beginthreadex(0, 0, recordProc, 0, 0, 0);

	return 0;
}
int RecordH264()
{
	convObj.Open("./video.flv", 0, 1);

	_beginthreadex(0, 0, encodeH264Proc, 0, 0, 0);
	_beginthreadex(0, 0, recordProc, 0, 0, 0);

	return 0;
}
int RecordAAC()
{
	convObj.Open("./audio.flv", 1, 0);

	_beginthreadex(0, 0, encodeAACProc, 0, 0, 0);
	_beginthreadex(0, 0, recordProc, 0, 0, 0);

	return 0;
}

#endif