#include "tsfile.h"
#include <string.h>

tsfilewriter::tsfilewriter()
{
	m_count = 0;
	m_audio_count = 0;
}

tsfilewriter::~tsfilewriter()
{

}

int tsfilewriter::open_file(const char* file_name)
{
	ts_file.open(file_name, std::ios::binary|std::ios::out);

	return ts_file.is_open();
}

int tsfilewriter::write_pat()
{
	static unsigned char pat[] = {
		0x47, 0x60, 0x00, 0x10, 0x00, 
		0x00, 0xB0, 0x0D, 0x00, 0x00, 0xC1, 0x00, 0x00, 
		0x00, 0x01, 0xE0, 0x81, 
		0x0C, 0x8C, 0xBE, 0x32
	};

	unsigned char temp_buffer[188];
	memset(temp_buffer,0xFF,sizeof(temp_buffer));
	memcpy(temp_buffer, pat, sizeof(pat));
	ts_file.write((const char*)temp_buffer, sizeof(temp_buffer));

	return 0;
}

int tsfilewriter::write_pmt()
{
/*	static unsigned char pmt[] = {
		0x47, 0x60, 0x81, 0x10, 0x00, 
		0x02, 0xB0, 0x17, 0x00, 0x01, 0xC1, 0x00, 0x00, 
		0xE8, 0x10, 0xF0, 0x00, 
		0x1B, 0xE8, 0x10, 0xF0, 0x00, 
		0x03, 0xE8, 0x14, 0xF0, 0x00, 
		0x66, 0x74, 0xA4, 0x2D
	};
*/
	static unsigned char pmt[] = {
		 0x47, 0x60, 0x81, 0x10, 0x00,
		 0x02, 0xB0, 0x17, 0x00, 0x01, 0xC1, 0x00, 0x00, 
		 0xE8, 0x10, 0xF0, 0x00, 
		 0x1B, 0xE8, 0x10, 0xF0, 0x00, 
		 0x0F, 0xE8, 0x14, 0xF0, 0x00, 
		 0x07, 0x69, 0x20, 0xA8
		};

	unsigned char temp_buffer[188];
	memset(temp_buffer,0xFF,sizeof(temp_buffer));
	memcpy(temp_buffer, pmt, sizeof(pmt));
	ts_file.write((const char*)temp_buffer, sizeof(temp_buffer));

	return 0;
}

int tsfilewriter::write_h264_pes(const char* framedata, int nFrameLen, uint64_t pts)
{
	int num = m_count;
	printf("******************************1*****%d ********%d\n",num ,m_count);
	int byte_s = 0;
	unsigned char ts_head[] = {0x47, 0x48, 0x10, 0x00}; //ts_head[3] b7-6:00保留，b5-4:01只有有效负荷，11即有有效负荷又有填充, 
	unsigned char ts_fill_0_6[] = {0x07, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00}; //此为pes有填充字段，一帧的开始与结束时需要填充；
	unsigned char pes_head[] = {0x00, 0x00, 0x00, 0x01, 0xE0};
	int len = nFrameLen;

	unsigned char pes_len[2] = {0};

	pes_len[0] = ((len <<16) >> 24);
	pes_len[1] =((len <<24) >> 24);

	unsigned char pts_field[] = {0x80,0x80, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00};
	unsigned char pcr[5] ={0};


	long PCR = pts<<7;
	pcr[0] =  ((PCR >> 32) & 0xFF);
	pcr[1] =  ((PCR >> 24) & 0xFF);
	pcr[2] =  ((PCR >> 16) & 0xFF);
	pcr[3] =  ((PCR >> 8) & 0xFF);
	pcr[4] =  (PCR & 0xFF);		

	for(int i = 0; i < 5 ; i++) {
		
		ts_fill_0_6[2+i] = pcr[i];
	}

	unsigned char  b4[4] = {0};		
	b4[0] = ((pts >> 24) & 0xFF);
	b4[1] =  (((pts<<8) >> 24) & 0xFF);
	b4[2] = ((pts<< 16)>>24 & 0xFF);
	b4[3] =  ((pts<<24)>>24 & 0xFF);

	unsigned char b1 = 0x00;
	b1 =(b4[0]>>5);
	b1 = (b1|0x01);
	b1 = (b1&0x2F);
	pts_field[3] = b1;
		
	b1 = 0x00;
	// pts的29-22
	b1 = ((b4[0] <<2)&0xFC);
	char  b2 = (((b4[1]>>6))&0x03);
	pts_field[4] = (b1|b2);

	// pts的21-15 第8位，标志位。
	b1 = ((b4[1]<<2) &0xFC);
	b2 = ((b4[2] >> 6)&0x03);
	pts_field[5] =  (b1|b2|0x01);
	
	b1 = ((b4[2] << 1)&0xFE);
	b2 = ((b4[3] >> 7)&0x01);
	
	pts_field[6] =  (b1 | b2);
	
	b1 = (b4[3] << 1);
	pts_field[7] =  (b1|0x01);

		// pes的构成是 TS-Head -> adaptation field->pes1 ts-head->pes2-(n-1)
		// ts-head->adaptation field->pes1

		// 注意在每一帧的视频帧被打包到pes的时候，其开头一定要加上 00 00 00 01 09 F0 这个暂时做为

		// 第一视频帧的头 TS-head数据的长度是4+自适应字段（pcr）14+6(00 00 00 01 09
		// xx),共为29个字节；188-29=159个字节；
		// 自适应字段的 7B+x = 1B：长度 + 1B:0x50 -含PCR，0x40-不含PCR；5B：PCR,
		// X:填充定符0xFF,只在最后帧时有效。

	// 写入第一个数据包的内容；
	unsigned  char tspkg[188] = {0};
	num = (num++)%16;
	ts_head[3] =( 0x30|((num<< 24)>>24)); // 生成连续记数器 即有有效负荷，又计数

	for(int i = 0; i < 4; i++) {
		
		tspkg[i] = ts_head[i];			
		byte_s++;
	}		

	//其中pes的头一共是15位；ts的头+补充字段一共为11位，前面加起来共为26位；余下的字段为188-26 = 162
	//如果b的长度少于162则需要在ts后面补充 162-b.length;			
	int m = 162 - len;		
	if(m > 0) {
		int k = 7 + m;
		ts_fill_0_6[0] = ((k<<24)>>24);
		//从第4个字符开始都置为FF
		for(int i = 0; i < k; i++) {
			
			tspkg[4+i] =  0xFF;
		}
		byte_s += m;	
	}

	for(int i = 4, j = 0; i < 11; i++, j++) {
				
			tspkg[i] = ts_fill_0_6[j];
			byte_s++;
	}	

	for(int i = 0; i < 5; i++) {
		
		tspkg[byte_s+i] = pes_head[i];
	}
	byte_s+=5;

	tspkg[byte_s] = pes_len[0];
	tspkg[byte_s+1] = pes_len[1];		
	byte_s +=2;
	
	for(int i = 0; i < 8; i++) {
		
		tspkg[byte_s+i] = pts_field[i];
	}
	byte_s+=8;

	//最后写入b中的数据；
	
	int d_len = 188 - byte_s;
	memcpy((tspkg+byte_s),framedata ,d_len);
	/*
	if(d_len <= len){
		memcpy((tspkg+byte_s),framedata ,d_len);
	}else{
		memcpy((tspkg+byte_s),(tspkg+byte_s),len);
		memset((tspkg+byte_s+len),1, d_len -len);
	}
	*/

	ts_file.write((char *)tspkg,188);

	//如果后续还有数据，则需要进行处理；		
	if(len > d_len) {
		
		int d_len1 = len - d_len;
		int m_num = d_len1 / 184;
		int n_num = d_len1 % 184;
		
		for(int i = 0; i < m_num; i++) {
		    	num = (num++)%16;
			unsigned char tspkg1[188] = {0};
			tspkg1[0] = 0x47;
			tspkg1[1] = 0x08;
			tspkg1[2] = 0x10;
			tspkg1[3] = (0x10|((num<< 24)>>24));   //仅含有效负荷，不含自适应区
			for(int j = 0; j < 184; j++) {
				
				tspkg1[4+j] = framedata[d_len+i*184+j];
			}
			ts_file.write((char*)tspkg1,188);
		}	
		if(n_num > 0) {
			num = (num++)%16;
			unsigned char tspkg2[188] = {0};
			
			tspkg2[0] = 0x47;
			tspkg2[1] = 0x08;
			tspkg2[2] = 0x10;
			tspkg2[3] =(0x30|((num<< 24)>>24));	//含有效负荷和自适应区,在此必需要确定没有PCR区；2
			
			int f_num = 184 - n_num;
			
			for(int i = 0; i < f_num; i++) {
				
				tspkg2[4+i] = 0xFF;
			}
			tspkg2[4] =  (f_num<<24>>24);
			tspkg2[5] = 0x40;//确定没有PCR区
							
			for(int i = 4+f_num, j=0; i < 188; i++, j++) {
		
				tspkg2[i] = framedata[d_len+m_num*184+j];
			}
			
			ts_file.write((char*)tspkg2,188);
		}
	}
	
	m_count = num;
	printf("******************************2*****%d ********%d\n",num ,m_count);

    return 0;  
}

int tsfilewriter::write_aac_pes(const char* framedata, int nFrameLen, uint64_t pts)
{
	int num = m_audio_count;
	printf("******************************1*****%d ********%d\n",num ,m_audio_count);
	int byte_s = 0;
	unsigned char ts_head[] = {0x47, 0x48, 0x14, 0x10}; //ts_head[3] b7-6:00保留，b5-4:01只有有效负荷，11即有有效负荷又有填充, 
	unsigned char pes_head[] = {0x00, 0x00, 0x01, 0xC0};
	int len = nFrameLen + 8;

	unsigned char pes_len[2] = {0};

	pes_len[0] = ((len <<16) >> 24);
	pes_len[1] =((len <<24) >> 24);

	unsigned char pts_field[] = {0x80,0x80, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00};
	unsigned char  b4[4] = {0};		
	b4[0] = ((pts >> 24) & 0xFF);
	b4[1] =  (((pts<<8) >> 24) & 0xFF);
	b4[2] = ((pts<< 16)>>24 & 0xFF);
	b4[3] =  ((pts<<24)>>24 & 0xFF);

	unsigned char b1 = 0x00;
	b1 =(b4[0]>>5);
	b1 = (b1|0x01);
	b1 = (b1&0x2F);
	pts_field[3] = b1;
		
	b1 = 0x00;
	// pts的29-22
	b1 = ((b4[0] <<2)&0xFC);
	char  b2 = (((b4[1]>>6))&0x03);
	pts_field[4] = (b1|b2);

	// pts的21-15 第8位，标志位。
	b1 = ((b4[1]<<2) &0xFC);
	b2 = ((b4[2] >> 6)&0x03);
	pts_field[5] =  (b1|b2|0x01);
	
	b1 = ((b4[2] << 1)&0xFE);
	b2 = ((b4[3] >> 7)&0x01);
	
	pts_field[6] =  (b1 | b2);
	
	b1 = (b4[3] << 1);
	pts_field[7] =  (b1|0x01);

		// pes的构成是 TS-Head -> adaptation field->pes1 ts-head->pes2-(n-1)
		// ts-head->adaptation field->pes1

		// 注意在每一帧的视频帧被打包到pes的时候，其开头一定要加上 00 00 00 01 09 F0 这个暂时做为

		// 第一视频帧的头 TS-head数据的长度是4+自适应字段（pcr）14+6(00 00 00 01 09
		// xx),共为29个字节；188-29=159个字节；
		// 自适应字段的 7B+x = 1B：长度 + 1B:0x50 -含PCR，0x40-不含PCR；5B：PCR,
		// X:填充定符0xFF,只在最后帧时有效。

	// 写入第一个数据包的内容；
	unsigned  char tspkg[188] = {0};
	num = (num++)%16;
	ts_head[3] =( 0x10|((num<< 24)>>24)); // 生成连续记数器 即有有效负荷，又计数

	for(int i = 0; i < sizeof(ts_head); i++) {
		
		tspkg[i] = ts_head[i];			
		byte_s++;
	}		

	//其中pes的头一共是15位；ts的头+补充字段一共为11位，前面加起来共为26位；余下的字段为188-26 = 162
	//如果b的长度少于162则需要在ts后面补充 162-b.length;			
	int m = 162 - len;		
	if(m > 0) {
		int k = m;
		//从第4个字符开始都置为FF
		for(int i = 0; i < k; i++) {
			
			tspkg[4+i] =  0xFF;
		}
		byte_s += m;	
	}

	for(int i = 0; i < sizeof(pes_head); i++) {
		
		tspkg[byte_s+i] = pes_head[i];
	}
	byte_s+=sizeof(pes_head);

	tspkg[byte_s] = pes_len[0];
	tspkg[byte_s+1] = pes_len[1];		
	byte_s +=2;
	
	for(int i = 0; i < 8; i++) {
		
		tspkg[byte_s+i] = pts_field[i];
	}
	byte_s+=8;

	//最后写入b中的数据；
	
	int d_len = 188 - byte_s;
	memcpy((tspkg+byte_s),framedata ,d_len);
	/*
	if(d_len <= len){
		memcpy((tspkg+byte_s),framedata ,d_len);
	}else{
		memcpy((tspkg+byte_s),(tspkg+byte_s),len);
		memset((tspkg+byte_s+len),1, d_len -len);
	}
	*/

	ts_file.write((char *)tspkg,188);

	//如果后续还有数据，则需要进行处理；		
	if(len > d_len) {
		
		int d_len1 = len - d_len;
		int m_num = d_len1 / 184;
		int n_num = d_len1 % 184;
		
		for(int i = 0; i < m_num; i++) {
		    	num = (num++)%16;
			unsigned char tspkg1[188] = {0};
			tspkg1[0] = 0x47;
			tspkg1[1] = 0x08;
			tspkg1[2] = 0x14;
			tspkg1[3] = (0x10|((num<< 24)>>24));   //仅含有效负荷，不含自适应区
			for(int j = 0; j < 184; j++) {
				
				tspkg1[4+j] = framedata[d_len+i*184+j];
			}
			ts_file.write((char*)tspkg1,188);
		}	
		if(n_num > 0) {
			num = (num++)%16;
			unsigned char tspkg2[188] = {0};
			
			tspkg2[0] = 0x47;
			tspkg2[1] = 0x08;
			tspkg2[2] = 0x14;
			tspkg2[3] =(0x10|((num<< 24)>>24));	//含有效负荷和自适应区,在此必需要确定没有PCR区；2
			
			int f_num = 184 - n_num;
			
			for(int i = 0; i < f_num; i++) {
				
				tspkg2[4+i] = 0xFF;
			}
			tspkg2[4] =  (f_num<<24>>24);
			tspkg2[5] = 0x40;//确定没有PCR区
							
			for(int i = 4+f_num, j=0; i < 188; i++, j++) {
		
				tspkg2[i] = framedata[d_len+m_num*184+j];
			}
			
			ts_file.write((char*)tspkg2,188);
		}
	}
	
	m_audio_count = num;
	printf("******************************2*****%d ********%d\n",num ,m_audio_count);

    return 0;  
}

int tsfilewriter::write_aac_pes2(const char* framedata, int nFrameLen, uint64_t pts)
{
	int num = m_audio_count;
	printf("******************************1*****%d ********%d\n",num ,m_audio_count);
	int byte_s = 0;
	unsigned char ts_head[] = {0x47, 0x48, 0x14, 0x00}; //ts_head[3] b7-6:00保留，b5-4:01只有有效负荷，11即有有效负荷又有填充, 
	unsigned char ts_fill_0_6[] = {0x07, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00}; //此为pes有填充字段，一帧的开始与结束时需要填充；
	unsigned char pes_head[] = {0x00, 0x00, 0x00, 0x01, 0xC0};
	int len = nFrameLen;

	unsigned char pes_len[2] = {0};

	pes_len[0] = ((len <<16) >> 24);
	pes_len[1] =((len <<24) >> 24);

	unsigned char pts_field[] = {0x80,0x80, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00};
	unsigned char pcr[5] ={0};

	long PCR = pts<<7;
	pcr[0] =  ((PCR >> 32) & 0xFF);
	pcr[1] =  ((PCR >> 24) & 0xFF);
	pcr[2] =  ((PCR >> 16) & 0xFF);
	pcr[3] =  ((PCR >> 8) & 0xFF);
	pcr[4] =  (PCR & 0xFF);		

	for(int i = 0; i < 5 ; i++) {
		
		ts_fill_0_6[2+i] = pcr[i];
	}

	unsigned char  b4[4] = {0};		
	b4[0] = ((pts >> 24) & 0xFF);
	b4[1] =  (((pts<<8) >> 24) & 0xFF);
	b4[2] = ((pts<< 16)>>24 & 0xFF);
	b4[3] =  ((pts<<24)>>24 & 0xFF);

	unsigned char b1 = 0x00;
	b1 =(b4[0]>>5);
	b1 = (b1|0x01);
	b1 = (b1&0x2F);
	pts_field[3] = b1;
		
	b1 = 0x00;
	// pts的29-22
	b1 = ((b4[0] <<2)&0xFC);
	char  b2 = (((b4[1]>>6))&0x03);
	pts_field[4] = (b1|b2);

	// pts的21-15 第8位，标志位。
	b1 = ((b4[1]<<2) &0xFC);
	b2 = ((b4[2] >> 6)&0x03);
	pts_field[5] =  (b1|b2|0x01);
	
	b1 = ((b4[2] << 1)&0xFE);
	b2 = ((b4[3] >> 7)&0x01);
	
	pts_field[6] =  (b1 | b2);
	
	b1 = (b4[3] << 1);
	pts_field[7] =  (b1|0x01);

		// pes的构成是 TS-Head -> adaptation field->pes1 ts-head->pes2-(n-1)
		// ts-head->adaptation field->pes1

		// 注意在每一帧的视频帧被打包到pes的时候，其开头一定要加上 00 00 00 01 09 F0 这个暂时做为

		// 第一视频帧的头 TS-head数据的长度是4+自适应字段（pcr）14+6(00 00 00 01 09
		// xx),共为29个字节；188-29=159个字节；
		// 自适应字段的 7B+x = 1B：长度 + 1B:0x50 -含PCR，0x40-不含PCR；5B：PCR,
		// X:填充定符0xFF,只在最后帧时有效。

	// 写入第一个数据包的内容；
	unsigned  char tspkg[188] = {0};
	num = (num++)%16;
	ts_head[3] =( 0x30|((num<< 24)>>24)); // 生成连续记数器 即有有效负荷，又计数

	for(int i = 0; i < 4; i++) {
		
		tspkg[i] = ts_head[i];			
		byte_s++;
	}		

	//其中pes的头一共是15位；ts的头+补充字段一共为11位，前面加起来共为26位；余下的字段为188-26 = 162
	//如果b的长度少于162则需要在ts后面补充 162-b.length;			
	int m = 162 - len;		
	if(m > 0) {
		int k = 7 + m;
		ts_fill_0_6[0] = ((k<<24)>>24);
		//从第4个字符开始都置为FF
		for(int i = 0; i < k; i++) {
			
			tspkg[4+i] =  0xFF;
		}
		byte_s += m;	
	}

	for(int i = 4, j = 0; i < 11; i++, j++) {
				
			tspkg[i] = ts_fill_0_6[j];
			byte_s++;
	}	

	for(int i = 0; i < 5; i++) {
		
		tspkg[byte_s+i] = pes_head[i];
	}
	byte_s+=5;

	tspkg[byte_s] = pes_len[0];
	tspkg[byte_s+1] = pes_len[1];		
	byte_s +=2;
	
	for(int i = 0; i < 8; i++) {
		
		tspkg[byte_s+i] = pts_field[i];
	}
	byte_s+=8;

	//最后写入b中的数据；
	
	int d_len = 188 - byte_s;
	memcpy((tspkg+byte_s),framedata ,d_len);
	/*
	if(d_len <= len){
		memcpy((tspkg+byte_s),framedata ,d_len);
	}else{
		memcpy((tspkg+byte_s),(tspkg+byte_s),len);
		memset((tspkg+byte_s+len),1, d_len -len);
	}
	*/

	ts_file.write((char *)tspkg,188);

	//如果后续还有数据，则需要进行处理；		
	if(len > d_len) {
		
		int d_len1 = len - d_len;
		int m_num = d_len1 / 184;
		int n_num = d_len1 % 184;
		
		for(int i = 0; i < m_num; i++) {
		    	num = (num++)%16;
			unsigned char tspkg1[188] = {0};
			tspkg1[0] = 0x47;
			tspkg1[1] = 0x08;
			tspkg1[2] = 0x10;
			tspkg1[3] = (0x10|((num<< 24)>>24));   //仅含有效负荷，不含自适应区
			for(int j = 0; j < 184; j++) {
				
				tspkg1[4+j] = framedata[d_len+i*184+j];
			}
			ts_file.write((char*)tspkg1,188);
		}	
		if(n_num > 0) {
			num = (num++)%16;
			unsigned char tspkg2[188] = {0};
			
			tspkg2[0] = 0x47;
			tspkg2[1] = 0x08;
			tspkg2[2] = 0x10;
			tspkg2[3] =(0x30|((num<< 24)>>24));	//含有效负荷和自适应区,在此必需要确定没有PCR区；2
			
			int f_num = 184 - n_num;
			
			for(int i = 0; i < f_num; i++) {
				
				tspkg2[4+i] = 0xFF;
			}
			tspkg2[4] =  (f_num<<24>>24);
			tspkg2[5] = 0x40;//确定没有PCR区
							
			for(int i = 4+f_num, j=0; i < 188; i++, j++) {
		
				tspkg2[i] = framedata[d_len+m_num*184+j];
			}
			
			ts_file.write((char*)tspkg2,188);
		}
	}
	
	m_audio_count = num;
	printf("******************************2*****%d ********%d\n",num ,m_audio_count);

    return 0;  
}

int tsfilewriter::close_file()
{
	ts_file.close();

	return 0;
}
