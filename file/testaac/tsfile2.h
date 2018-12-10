#ifndef __TSFILE2_H__
#define __TSFILE2_H__

#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <stdint.h>
#include "tsmux.h"

#define BUF_SIZE (1<<20)

class tsfilewriter2 
{
public:
	tsfilewriter2();
	virtual ~tsfilewriter2();

	int open_file(const char* file_name);
	int close_file();

	int write_h264_pes(const char* framedata, int nFrameLen, uint64_t pts);
	int write_aac_pes(const char* framedata, int nFrameLen, uint64_t pts);
private:
	std::fstream ts_file;

	TsProgramInfo m_prog_info;
	int m_frame_count;
	uint8_t m_outbuf[BUF_SIZE];
};

#endif
