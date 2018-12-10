#include "tsfile2.h"
#include <string.h>

tsfilewriter2::tsfilewriter2()
{
	memset(&m_prog_info, 0, sizeof(m_prog_info));

//	m_prog_info.program_num = 1;
//	m_prog_info.prog[0].stream_num = 1;
//	m_prog_info.prog[0].stream[0].type = STREAM_TYPE_VIDEO_H264;

	m_prog_info.program_num = 1;
	m_prog_info.prog[0].stream_num = 2;
	m_prog_info.prog[0].stream[0].type = STREAM_TYPE_VIDEO_H264;
	m_prog_info.prog[0].stream[1].type = STREAM_TYPE_AUDIO_AAC;
}

tsfilewriter2::~tsfilewriter2()
{
	m_frame_count = 0;

	ts_file.close();
}

int tsfilewriter2::open_file(const char* file_name)
{
	m_frame_count = 0;

	ts_file.open(file_name, std::ios::binary|std::ios::out);

	return ts_file.is_open();
}

int tsfilewriter2::write_h264_pes(const char* framedata, int nFrameLen, uint64_t pts)
{
	TEsFrame es = {0};
	es.program_number = 0;
	es.stream_number = 0;
	es.frame = (uint8_t*)framedata;
	es.length = nFrameLen;
	// 示例中按帧率为25fps累计时间戳。正式使用应根据帧实际的时间戳填写。
	es.pts = pts;		
	es.is_key = 0;//framelength < 100? 1:0;					// 这里简单处理，认为信息帧（非数据帧）为关键帧。
	if( m_frame_count%100 == 0 )
		es.is_key = 1;
	m_frame_count++;
	
	es.ps_pes_length = 8000;

//	printf("framelength %d \n",length);
	int outlen = lts_ts_stream(&es, m_outbuf, BUF_SIZE, &m_prog_info);
	if (outlen <= 0)
		return -1;

	ts_file.write((const char*)m_outbuf, outlen);

    return 0;  
}

int tsfilewriter2::write_aac_pes(const char* framedata, int nFrameLen, uint64_t pts)
{
	TEsFrame es = {0};
	es.program_number = 0;
	es.stream_number = 1;
	es.frame = (uint8_t*)framedata;
	es.length = nFrameLen;
	es.pts = pts;		// 示例中按帧率为25fps累计时间戳。正式使用应根据帧实际的时间戳填写。
	es.is_key = 0;//framelength < 100? 1:0;					// 这里简单处理，认为信息帧（非数据帧）为关键帧。

//	if( m_frame_count%100 == 0 )
//		es.is_key = 1;
//	m_frame_count++;
	
	es.ps_pes_length = 8000;

//	printf("framelength %d \n",length);
	int outlen = lts_ts_stream(&es, m_outbuf, BUF_SIZE, &m_prog_info);
	if (outlen <= 0)
		return -1;

	ts_file.write((const char*)m_outbuf, outlen);

    return 0;  
}

int tsfilewriter2::close_file()
{
	ts_file.close();

	return 0;
}
