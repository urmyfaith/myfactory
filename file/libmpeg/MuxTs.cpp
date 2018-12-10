/*
author:yume.liu
date:2016.7.21
muxing h265/hevc,h264,aac to .ts
https://github.com/yu-s/MuxTs
*/
#include "MuxTs.h"

using namespace std;

static uint32_t crc32table[256] = {
	0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
	0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
	0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
	0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
	0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
	0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
	0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
	0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
	0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
	0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
	0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
	0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
	0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
	0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
	0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
	0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
	0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
	0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
	0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
	0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
	0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
	0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
	0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
	0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
	0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
	0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
	0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
	0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
	0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
	0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
	0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
	0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
	0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};


uint32_t CRC32(const uint8_t *data, int len)
{
	int i;
	uint32_t crc = 0xFFFFFFFF;
	for (i = 0; i < len; i++)
		crc = (crc << 8) ^ crc32table[((crc >> 24) ^ *data++) & 0xFF];
	return crc;
}


MuxTs::MuxTs()
{
}


MuxTs::~MuxTs()
{
}

int MuxTs::CreateFile(const char* filename)
{
	if (NULL == filename)
	{
		return MUXTS_ERROR_FILE_NULL;
	}
	int namelen = strlen(filename);
	if (!((filename[namelen - 1] == 'S') || (filename[namelen - 1] == 's')
		&& (filename[namelen - 2] == 'T') || (filename[namelen - 2] == 't')
		&& (filename[namelen - 3] == '.')))
	{
		return MUXTS_ERROR_FILE_NOTTS;
	}
	data.buff = new BYTE[MUXTS_TS_PACKET_SIZE];
	data.seek = 0;
	m_ofile.open(filename, ios::out | ios::binary | ios::trunc);
	//init pat
	m_pat.PID = 0;
	m_pat.count = 0;
	m_pat.PMT_Quantity = 0;
	//init
	m_packetCount = 0;
	vector<TS_PAT_Program> _pmt_s;
	_pmt.swap(_pmt_s);
	vector<TS_PMT_Stream> _pes_s;
	_pes.swap(_pes_s);
	return 0;
}

int MuxTs::CloseFile()
{
	delete[] data.buff;
	m_ofile.close();
	return 0;
}

int MuxTs::AddNewProgram(unsigned PID, unsigned number)
{
	m_pat.pmt.PID = PID;
	m_pat.pmt.count = 0;
	m_pat.pmt.PES_Quantity = 0;
	m_pat.pmt.program_number = number;
	return 0;
}

int MuxTs::AddNewStream(unsigned PID, AVCODEC codec)
{
	INFO_PES pes;
	pes.PID = PID;
	pes.count = 0;
	switch (codec)
	{
	case MUXTS_CODEC_HEVC:
		pes.stream_id = 0xe0;
		pes.aorv = MUXTS_TYPE_VIDEO;
		pes.stream_type = 0x24;
		pes_index_video = m_pat.pmt.pes.size();
		break;
	case MUXTS_CODEC_H264:
		pes.stream_id = 0xe0;
		pes.aorv = MUXTS_TYPE_VIDEO;
		pes.stream_type = 0x1b;
		pes_index_video = m_pat.pmt.pes.size();
		break;
	case MUXTS_CODEC_AAC:
		pes.stream_id = 0xc0;
		pes.aorv = MUXTS_TYPE_AUDIO;
		pes.stream_type = 0x0f;
		pes_index_audio = m_pat.pmt.pes.size();
		break;
	default:
		return MUXTS_ERRTR_CODEC_NOTUSE;
		break;
	}
	m_pat.pmt.PES_Quantity++;
	m_pat.pmt.pes.push_back(pes);
	return 0;
}

int MuxTs::WriteFrame(unsigned char * buffer, int len, uint64_t pts, AVCODEC type)
{
	unsigned char *pbuf = buffer;
	int length = len;
	unsigned pesLen = 0;
	bool havePesHeader = false;
	unsigned index;
	if (MUXTS_CODEC_H264 == type || MUXTS_CODEC_HEVC == type)
	{
		index = pes_index_video;
	}
	else if (MUXTS_CODEC_AAC == type)
	{
		index = pes_index_audio;
		pesLen = length + 8; //pes音频包需要指定长度，长度的值为该属性后的所有字节个数，包括音频编码和部分描述字节，这里指定为8
	}
	while (length > 0)
	{
		if (0 == m_packetCount)
		{
			//set _pmt
			TS_PAT_Program _pgm;
			_pgm.program_number = m_pat.pmt.program_number;
			_pgm.program_map_PID = m_pat.pmt.PID;
			_pgm.reserved_3 = 0xff;
			_pmt.push_back(_pgm);
			//set _pes
			for (unsigned i = 0; i < m_pat.pmt.PES_Quantity; i++)
			{
				TS_PMT_Stream _st;
				_st.elementary_PID = m_pat.pmt.pes[i].PID;
				_st.stream_type = m_pat.pmt.pes[i].stream_type;
				_st.ES_info_length = 0x00;
				_st.reserved_5 = 0xff;
				_st.reserved_6 = 0xff;
				_pes.push_back(_st);
			}
		}
		//insert pat packet and pmt packet
		if (0 == m_packetCount%MUXTS_PAT_SPACEPAT)
		{
			this->WritePacketHeader(m_pat.PID, 0x01, 0x01, m_pat.count);
			this->WritePAT(_pmt, true);
			INCOUNT(m_pat.count);
			m_packetCount++;
			this->WritePacketHeader(m_pat.pmt.PID, 0x01, 0x01, m_pat.pmt.count);
			this->WritePMT(_pes, m_pat.pmt.program_number, true);
			INCOUNT(m_pat.pmt.count);
			m_packetCount++;
		}
		else if (!havePesHeader)
		{
			havePesHeader = true;
			this->WritePacketHeader(m_pat.pmt.pes[index].PID, 0x01, 0x03, m_pat.pmt.pes[index].count);
			TS_PCR pcr;
			pcr.program_clock_reference_base = pts;
			pcr.reserved_1 = 0xff;
			pcr.program_clock_reference_extension = 0x00;
			//AAC need PCR?
			if (MUXTS_CODEC_AAC == type)
			{
				this->WriteAdaptation(0x01, false, pcr);
			}
			else
			{
				this->WriteAdaptation(0x07, true, pcr);
			}
			TS_PES_PTS s_pts;
			s_pts.head = 0x02;
			s_pts.resvered_1 = 0xff;
			s_pts.resvered_2 = 0xff;
			s_pts.resvered_3 = 0xff;
			s_pts.pts1 = pts >> 30;
			s_pts.pts2 = pts >> 15;
			s_pts.pts3 = pts;
			if (MUXTS_CODEC_AAC == type)
			{
				this->WritePES(pesLen, m_pat.pmt.pes[index].stream_id, 0x02, 0x05, s_pts);
			}
			else if (MUXTS_CODEC_H264 == type || MUXTS_CODEC_HEVC == type)
			{
				this->WritePES(pesLen, m_pat.pmt.pes[index].stream_id, 0x02, 0x05, s_pts);
			}
			INCOUNT(m_pat.pmt.pes[index].count);
			m_packetCount++;
			//填充码流数据
			if (length >= MUXTS_TS_PACKET_SIZE - data.seek)
			{
				memcpy(data.buff + data.seek, pbuf, MUXTS_TS_PACKET_SIZE - data.seek);
				pbuf += MUXTS_TS_PACKET_SIZE - data.seek;
				length -= MUXTS_TS_PACKET_SIZE - data.seek;
				data.seek += MUXTS_TS_PACKET_SIZE - data.seek;
			}
			this->Output2File();
		}
		else
		{
			if (length >= MUXTS_TS_PACKET_SIZE - 4)
			{
				this->WritePacketHeader(m_pat.pmt.pes[index].PID, 0x00, 0x01, m_pat.pmt.pes[index].count);
				memcpy(data.buff + data.seek, pbuf, MUXTS_TS_PACKET_SIZE - data.seek);
				pbuf += MUXTS_TS_PACKET_SIZE - data.seek;
				length -= MUXTS_TS_PACKET_SIZE - data.seek;
				data.seek += MUXTS_TS_PACKET_SIZE - data.seek;
			}
			else
			{
				this->WritePacketHeader(m_pat.pmt.pes[index].PID, 0x00, 0x03, m_pat.pmt.pes[index].count);
				this->WriteAdaptation(MUXTS_TS_PACKET_SIZE - length - 5, false, TS_PCR());
				memcpy(data.buff + data.seek, pbuf, MUXTS_TS_PACKET_SIZE - data.seek);
				pbuf += MUXTS_TS_PACKET_SIZE - data.seek;
				length -= MUXTS_TS_PACKET_SIZE - data.seek;
				data.seek += MUXTS_TS_PACKET_SIZE - data.seek;
			}
			INCOUNT(m_pat.pmt.pes[index].count);
			m_packetCount++;
			this->Output2File();
		}
	}
	return 0;
}

int MuxTs::WritePacketHeader(unsigned PID, unsigned payload_unit_start_indicator, unsigned adaptation_field_control, unsigned continuity_counter)
{
	TS_HEADER ts_header;
	ts_header.sync_byte = 0x47;
	ts_header.transport_error_indicator = 0x00;
	ts_header.payload_unit_start_indicator = payload_unit_start_indicator;
	ts_header.transport_priority = 0x00;
	ts_header.PID = PID;
	ts_header.transport_scrambling_control = 0x00;
	ts_header.adaptation_field_control = adaptation_field_control;
	ts_header.continuity_counter = continuity_counter;
	BYTE4 bit4 = 0x00;
	BITSET(bit4, 0, ts_header.sync_byte);
	BITSET(bit4, 1, ts_header.transport_error_indicator);
	BITSET(bit4, 1, ts_header.payload_unit_start_indicator);
	BITSET(bit4, 1, ts_header.transport_priority);
	BITSET(bit4, 13, ts_header.PID);
	BITSET(bit4, 2, ts_header.transport_scrambling_control);
	BITSET(bit4, 2, ts_header.adaptation_field_control);
	BITSET(bit4, 4, ts_header.continuity_counter);
	ret = this->SwapBigLittleEndian((char*)&bit4, 32);
	if (ret < 0)
	{
		return ret;
	}
	memcpy(data.buff, &bit4, 4);
	data.seek = 4;
	if (0x01 == payload_unit_start_indicator && 0x01 == adaptation_field_control)
	{
		const char p = 0x00;
		//m_ofile.write(&p, 1);
		memcpy(data.buff+data.seek, &p, 1);
		data.seek++;
	}
	return 0;
}

int MuxTs::WritePAT(std::vector<TS_PAT_Program>& program, bool has_payload_unit_start_indicator)
{
	if (program.empty())
	{
		return MUXTS_ERRTR_PAT_NOPROGRAM;
	}
	int CRC32begin = 0;
	TS_PAT ts_pat;
	ts_pat.table_id = 0x00;
	ts_pat.section_syntax_indicator = 0xff;
	ts_pat.zero = 0x00;
	ts_pat.reserved_1 = 0xff;
	unsigned length = 8;
	if (has_payload_unit_start_indicator)
	{
		CRC32begin = 5;
		length++;
	}
	else
	{
		CRC32begin = 4;
	}
	length += 4 * program.size();
	ts_pat.section_length = length;
	ts_pat.transport_stream_id = 0x0001;
	ts_pat.reserved_2 = 0xff;
	ts_pat.version_number = 0x00;
	ts_pat.current_next_indicator = 0xff;
	ts_pat.section_number = 0x00;
	ts_pat.last_section_number = 0x00;
	ts_pat.program = program;
	ts_pat.CRC_32 = 1;
	BYTE8 bit8 = 0x00;
	BITSET(bit8, 0, ts_pat.table_id);
	BITSET(bit8, 1, ts_pat.section_syntax_indicator);
	BITSET(bit8, 1, ts_pat.zero);
	BITSET(bit8, 2, ts_pat.reserved_1);
	BITSET(bit8, 12, ts_pat.section_length);
	BITSET(bit8, 16, ts_pat.transport_stream_id);
	BITSET(bit8, 2, ts_pat.reserved_2);
	BITSET(bit8, 5, ts_pat.version_number);
	BITSET(bit8, 1, ts_pat.current_next_indicator);
	BITSET(bit8, 8, ts_pat.section_number);
	BITSET(bit8, 8, ts_pat.last_section_number);
	ret = this->SwapBigLittleEndian((char*)&bit8, 64);
	if (ret < 0)
	{
		return ret;
	}
	memcpy(data.buff + data.seek, &bit8, 8);
	data.seek += 8;
	vector<TS_PAT_Program>::iterator iter;
	for (iter = program.begin(); iter != program.end(); ++iter)
	{
		BYTE4 bit4 = 0x00;
		BITSET(bit4, 0, iter->program_number);
		BITSET(bit4, 3, iter->reserved_3);
		BITSET(bit4, 13, iter->program_map_PID);
		ret = this->SwapBigLittleEndian((char*)&bit4, 32);
		if (ret < 0)
		{
			return ret;
		}
		memcpy(data.buff + data.seek, &bit4, 4);
		data.seek += 4;
	}
	ret = this->SetCRC32(CRC32begin);
	if (ret < 0)
	{
		return ret;
	}
	ret = this->FillPayload(MUXTS_TS_PACKET_SIZE - data.seek);
	if (ret < 0)
	{
		return ret;
	}
	this->Output2File();
	return 0;
}

int MuxTs::WritePMT(std::vector<TS_PMT_Stream> PMT_Stream, int program_number, bool has_payload_unit_start_indicator)
{
	if (PMT_Stream.empty())
	{
		return MUXTS_ERRTR_PMT_NOSTREAM;
	}
	int CRC32begin = 0;
	TS_PMT ts_pmt;
	ts_pmt.table_id = 0x02;
	ts_pmt.section_syntax_indicator = 0x01;
	ts_pmt.zero = 0x00;
	ts_pmt.reserved_1 = 0xff;
	unsigned length = 12;
	if (has_payload_unit_start_indicator)
	{
		CRC32begin = 5;
		length++;
	}
	else
	{
		CRC32begin = 4;
	}
	length += 5 * PMT_Stream.size();
	ts_pmt.section_length = length;
	ts_pmt.program_number = program_number;
	ts_pmt.reserved_2 = 0xff;
	ts_pmt.version_number = 0x00;
	ts_pmt.current_next_indicator = 0xff;
	ts_pmt.section_number = 0x00;
	ts_pmt.last_section_number = 0x00;
	ts_pmt.reserved_3 = 0xff;
	ts_pmt.PCR_PID = 0x100;
	ts_pmt.reserved_4 = 0xff;
	ts_pmt.program_info_length = 0x00;
	ts_pmt.PMT_Stream = PMT_Stream;
	ts_pmt.CRC_32 = 1;
	//1-8bit
	BYTE8 bit8 = 0x00;
	BITSET(bit8, 0, ts_pmt.table_id);
	BITSET(bit8, 1, ts_pmt.section_syntax_indicator);
	BITSET(bit8, 1, ts_pmt.zero);
	BITSET(bit8, 2, ts_pmt.reserved_1);
	BITSET(bit8, 12, ts_pmt.section_length);
	BITSET(bit8, 16, ts_pmt.program_number);
	BITSET(bit8, 2, ts_pmt.reserved_2);
	BITSET(bit8, 5, ts_pmt.version_number);
	BITSET(bit8, 1, ts_pmt.current_next_indicator);
	BITSET(bit8, 8, ts_pmt.section_number);
	BITSET(bit8, 8, ts_pmt.last_section_number);
	ret = this->SwapBigLittleEndian((char*)&bit8, 64);
	if (ret < 0)
	{
		return ret;
	}
	memcpy(data.buff + data.seek, &bit8, 8);
	data.seek += 8;
	//9-12bit
	BYTE4 bit4 = 0x00;
	BITSET(bit4, 0, ts_pmt.reserved_3);
	BITSET(bit4, 13, ts_pmt.PCR_PID);
	BITSET(bit4, 4, ts_pmt.reserved_4);
	BITSET(bit4, 12, ts_pmt.program_info_length);
	ret = this->SwapBigLittleEndian((char*)&bit4, 32);
	if (ret < 0)
	{
		return ret;
	}
	memcpy(data.buff + data.seek, &bit4, 4);
	data.seek += 4;
	//PMT_Stream
	vector<TS_PMT_Stream>::iterator iter;
	for (iter = PMT_Stream.begin(); iter != PMT_Stream.end(); ++iter)
	{
		BYTE bit = 0x00;
		BITSET(bit, 0, iter->stream_type);
		memcpy(data.buff + data.seek, &bit, 1);
		data.seek += 1;
		BYTE4 bit4_f = 0x00;
		BITSET(bit4_f, 0, iter->reserved_5);
		BITSET(bit4_f, 13, iter->elementary_PID);
		BITSET(bit4_f, 4, iter->reserved_6);
		BITSET(bit4_f, 12, iter->ES_info_length);
		ret = this->SwapBigLittleEndian((char*)&bit4_f, 32);
		if (ret < 0)
		{
			return ret;
		}
		memcpy(data.buff + data.seek, &bit4_f, 4);
		data.seek += 4;
	}
	ret = this->SetCRC32(CRC32begin);
	if (ret < 0)
	{
		return ret;
	}
	ret = this->FillPayload(MUXTS_TS_PACKET_SIZE - data.seek);
	if (ret < 0)
	{
		return ret;
	}
	this->Output2File();
	return 0;
}

int MuxTs::WriteAdaptation(unsigned length, bool PCR_flag, TS_PCR pcr)
{
	TS_ADAPTATION ts_adap;
	ts_adap.adaptation_field_length = length;
	ts_adap.discontinuity_indicator = 0x00;
	ts_adap.elementary_stream_priority_indicator = 0x00;
	if (PCR_flag)
	{
		ts_adap.PCR_flag = 0xff;
		ts_adap.random_access_indicator = 0x00;
		ts_adap.PCR = pcr;
	}
	else
	{
		ts_adap.PCR_flag = 0x00;
		ts_adap.random_access_indicator = 0x00;
	}
	//aac音频时的情况 此处可能会出错
	if (0x01 == length)
	{
		ts_adap.random_access_indicator = 0x00;//aac ff
	}
	ts_adap.OPCR_flag = 0x00;
	ts_adap.splicing_point_flag = 0x00;
	ts_adap.transport_private_data_flag = 0x00;
	ts_adap.adaptation_field_extension_flag = 0x00;
	BYTE bit1_len = 0x00;
	BITSET(bit1_len, 0, ts_adap.adaptation_field_length);
	memcpy(data.buff + data.seek, &bit1_len, 1);
	data.seek += 1;
	if (0 == ts_adap.adaptation_field_length)
	{
		return 0;
	}
	BYTE bit1 = 0x00;
	BITSET(bit1, 0, ts_adap.discontinuity_indicator);
	BITSET(bit1, 1, ts_adap.random_access_indicator);
	BITSET(bit1, 1, ts_adap.elementary_stream_priority_indicator);
	BITSET(bit1, 1, ts_adap.PCR_flag);
	BITSET(bit1, 1, ts_adap.OPCR_flag);
	BITSET(bit1, 1, ts_adap.splicing_point_flag);
	BITSET(bit1, 1, ts_adap.transport_private_data_flag);
	BITSET(bit1, 1, ts_adap.adaptation_field_extension_flag);
	memcpy(data.buff + data.seek, &bit1, 1);
	data.seek += 1;
	//PCR
	if (PCR_flag)
	{
		BYTE bit_pcr_1 = 0x00;
		bit_pcr_1 |= ts_adap.PCR.program_clock_reference_base >> 25;
		memcpy(data.buff + data.seek, &bit_pcr_1, 1);
		data.seek += 1;
		BYTE4 bit_pcr_4 = 0x00;
		bit_pcr_4 |= ts_adap.PCR.program_clock_reference_base & 0x1ffffff;
		BITSET(bit_pcr_4, 6, ts_adap.PCR.reserved_1);
		bit_pcr_4 <<= 1;
		bit_pcr_4 |= ts_adap.PCR.program_clock_reference_extension & 0x100;
		ret = this->SwapBigLittleEndian((char*)&bit_pcr_4, 32);
		if (ret < 0)
		{
			return ret;
		}
		memcpy(data.buff + data.seek, &bit_pcr_4, 4);
		data.seek += 4;
		BYTE bit_pcr_2 = 0x00;
		bit_pcr_2 |= ts_adap.PCR.program_clock_reference_extension;
		memcpy(data.buff + data.seek, &bit_pcr_2, 1);
		data.seek += 1;
	}
	else
	{
		unsigned short int payloadLen = ts_adap.adaptation_field_length - 1;
		this->FillPayload((int)payloadLen);
	}
	
	return 0;
}

int MuxTs::WritePES(unsigned length, unsigned stream_id, unsigned PTS_DTS_flag, unsigned data_length, TS_PES_PTS ts_pts)
{
	TS_PES ts_pes;
	ts_pes.packet_start_code_prefix = 0x000001;
	ts_pes.stream_id = stream_id;
	ts_pes.PESPacket_length = length;
	ts_pes.marker_bits = 0x02;
	ts_pes.PES_scrambling_control = 0x00;
	ts_pes.PES_priority = 0x00;
	ts_pes.data_alignment_indicator = 0x00;
	ts_pes.copyright = 0x00;
	ts_pes.original_or_copy = 0x00;
	ts_pes.PTS_DTS_flags = PTS_DTS_flag;
	ts_pes.ESCR_flag = 0x00;
	ts_pes.ES_rate_flag = 0x00;
	ts_pes.DSM_trick_mode_flag = 0x00;
	ts_pes.additional_copy_info_flag = 0x00;
	ts_pes.PES_CRC_flag = 0x00;
	ts_pes.PES_extension_flag = 0x00;
	ts_pes.PES_header_data_length = data_length;
	BYTE8 bit8 = 0x00;
	BITSET(bit8, 0, ts_pes.packet_start_code_prefix);
	BITSET(bit8, 8, ts_pes.stream_id);
	BITSET(bit8, 16, ts_pes.PESPacket_length);
	BITSET(bit8, 2, ts_pes.marker_bits);
	BITSET(bit8, 2, ts_pes.PES_scrambling_control);
	BITSET(bit8, 1, ts_pes.PES_priority);
	BITSET(bit8, 1, ts_pes.data_alignment_indicator);
	BITSET(bit8, 1, ts_pes.copyright);
	BITSET(bit8, 1, ts_pes.original_or_copy);
	BITSET(bit8, 2, ts_pes.PTS_DTS_flags);
	BITSET(bit8, 1, ts_pes.ESCR_flag);
	BITSET(bit8, 1, ts_pes.ES_rate_flag);
	BITSET(bit8, 1, ts_pes.DSM_trick_mode_flag);
	BITSET(bit8, 1, ts_pes.additional_copy_info_flag);
	BITSET(bit8, 1, ts_pes.PES_CRC_flag);
	BITSET(bit8, 1, ts_pes.PES_extension_flag);
	ret = this->SwapBigLittleEndian((char*)&bit8, 64);
	if (ret < 0)
	{
		return ret;
	}
	memcpy(data.buff + data.seek, &bit8, 8);
	data.seek += 8;
	BYTE bit1 = 0x00;
	bit1 |= ts_pes.PES_header_data_length;
	memcpy(data.buff + data.seek, &bit1, 1);
	data.seek += 1;
	//PTS
	if (0x02 == ts_pes.PTS_DTS_flags && 0x05 == ts_pes.PES_header_data_length)
	{
		BYTE bit_pts1 = 0x00;
		BITSET(bit_pts1, 0, ts_pts.head);
		BITSET(bit_pts1, 3, ts_pts.pts1);
		BITSET(bit_pts1, 1, ts_pts.resvered_1);
		memcpy(data.buff + data.seek, &bit_pts1, 1);
		data.seek += 1;
		BYTE4 bit_pts4 = 0x00;
		BITSET(bit_pts4, 0, ts_pts.pts2);
		BITSET(bit_pts4, 1, ts_pts.resvered_2);
		BITSET(bit_pts4, 15, ts_pts.pts3);
		BITSET(bit_pts4, 1, ts_pts.resvered_3);
		ret = this->SwapBigLittleEndian((char*)&bit_pts4, 32);
		if (ret < 0)
		{
			return ret;
		}
		memcpy(data.buff + data.seek, &bit_pts4, 4);
		data.seek += 4;
	}
	//写入DTS，值等于PTS
#ifdef MUXTS_ENABLE_WRITE_DTS
	if (0x03 == ts_pes.PTS_DTS_flags && 0x0a == ts_pes.PES_header_data_length)
	{
		BYTE bit_pts1 = 0x00;
		ts_pts.head = 0x03;
		BITSET(bit_pts1, 0, ts_pts.head);
		BITSET(bit_pts1, 3, ts_pts.pts1);
		BITSET(bit_pts1, 1, ts_pts.resvered_1);
		memcpy(data.buff + data.seek, &bit_pts1, 1);
		data.seek += 1;
		BYTE4 bit_pts4 = 0x00;
		BITSET(bit_pts4, 0, ts_pts.pts2);
		BITSET(bit_pts4, 1, ts_pts.resvered_2);
		BITSET(bit_pts4, 15, ts_pts.pts3);
		BITSET(bit_pts4, 1, ts_pts.resvered_3);
		ret = this->SwapBigLittleEndian((char*)&bit_pts4, 32);
		if (ret < 0)
		{
			return ret;
		}
		memcpy(data.buff + data.seek, &bit_pts4, 4);
		data.seek += 4;
	}
	data.buff[data.seek] = 0x11;
	data.seek++;
	memcpy(data.buff + data.seek, data.buff + data.seek - 5, 4);
	data.seek += 4;
#endif // MUXTS_ENABLE_WRITE_DTS

	//h264流需要加前缀？
	if (0xe0 == stream_id && 0x1b == m_pat.pmt.pes[pes_index_video].stream_type)
	{
		BYTE _nonBit = 0x00;
		memcpy(data.buff + data.seek, &_nonBit, 1);
		data.seek += 1;
		memcpy(data.buff + data.seek, &_nonBit, 1);
		data.seek += 1;
		memcpy(data.buff + data.seek, &_nonBit, 1);
		data.seek += 1;
		_nonBit = 0x01;
		memcpy(data.buff + data.seek, &_nonBit, 1);
		data.seek += 1;
		_nonBit = 0x09;
		memcpy(data.buff + data.seek, &_nonBit, 1);
		data.seek += 1;
		_nonBit = 0xf0;
		memcpy(data.buff + data.seek, &_nonBit, 1);
		data.seek += 1;
	}
	return 0;
}

int MuxTs::SwapBigLittleEndian(char * ptr, int bitlen)
{
#ifdef MUXTS_ENABLE_SWAP_ENDIAN32
	if (32 == bitlen)
	{
		swap(ptr[0], ptr[3]);
		swap(ptr[1], ptr[2]);
		return 0;
	}
#endif // MUXTS_ENABLE_SWAP_ENDIAN32
#ifdef MUXTS_ENABLE_SWAP_ENDIAN64
	if (64 == bitlen)
	{
		swap(ptr[0], ptr[7]);
		swap(ptr[1], ptr[6]);
		swap(ptr[2], ptr[5]);
		swap(ptr[3], ptr[4]);
		return 0;
	}
#endif // MUXTS_ENABLE_SWAP_ENDIAN64

	return 0;
}

int MuxTs::SetCRC32(int begin)
{
	uint32_t crc32 = CRC32(data.buff+begin, data.seek-begin);
	//uint32_t crc32 = av_crc(crc32table, 0, data.buff+5, data.seek);
	ret = this->SwapBigLittleEndian((char*)&crc32, 32);
	if (ret < 0)
	{
		return ret;
	}
	memcpy(data.buff + data.seek, &crc32, 4);
	data.seek += 4;
	return 0;
}

int MuxTs::FillPayload(int count)
{
	if ((data.seek + count) > MUXTS_TS_PACKET_SIZE)
	{
		return MUXTS_ERROR_FILLPAYLOAD_OVER;
	}
	for (int i = 0; i < count; i++)
	{
		*(data.buff+data.seek) = 0xff;
		data.seek++;
	}
	return 0;
}

int MuxTs::Output2File()
{
	if (data.seek != MUXTS_TS_PACKET_SIZE)
	{
		return MUXTS_ERROR_FILLPAYLOAD_OVER;
	}
	m_ofile.write((char*)data.buff, data.seek);
	data.seek = 0;
	return 0;
}
