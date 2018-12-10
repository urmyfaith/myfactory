#ifndef __TSFILE_H__
#define __TSFILE_H__

#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <stdint.h>

class tsfilewriter 
{
public:
	tsfilewriter();
	virtual ~tsfilewriter();

	int open_file(const char* file_name);
	int close_file();

	int write_pat();
	int write_pmt();
	int write_h264_pes(const char* framedata, int nFrameLen, uint64_t pts);
	int write_aac_pes(const char* framedata, int nFrameLen, uint64_t pts);
	int write_aac_pes2(const char* framedata, int nFrameLen, uint64_t pts);
private:
	std::fstream ts_file;
	uint32_t m_count;
	uint32_t m_audio_count;
};

#endif
