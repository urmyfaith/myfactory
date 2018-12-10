#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fstream>

#include <string>

#include "typedefs.h"
#include "demux.h"

enum mp4_track_type {
	MP4_TRACK_TYPE_UNKNOWN = 0,
	MP4_TRACK_TYPE_VIDEO,
	MP4_TRACK_TYPE_AUDIO,
	MP4_TRACK_TYPE_HINT,
	MP4_TRACK_TYPE_METADATA,
	MP4_TRACK_TYPE_TEXT,
	MP4_TRACK_TYPE_CHAPTERS,

	MP4_TRACK_TYPE_MAX,
};

enum mp4_video_codec {
	MP4_VIDEO_CODEC_UNKNOWN = 0,
	MP4_VIDEO_CODEC_AVC,

	MP4_VIDEO_CODEC_MAX,
};

enum mp4_audio_codec {
	MP4_AUDIO_CODEC_UNKNOWN = 0,
	MP4_AUDIO_CODEC_AAC,

	MP4_AUDIO_CODEC_MAX,
};

uint64_t getonebox(const char* buffer, int maxBytes, struct mp4_box_header_t *box)
{
//	printf("mp4_demux_parse_moov \n");

	/* box size */
	uint32_t val32 = *(uint32_t*)buffer;
	buffer += sizeof(val32);
	int boxReadBytes = sizeof(val32);
	size_t _count = ntohl(val32);

	box->size = ntohl(val32);

	/* box type */
	val32 = *(uint32_t*)buffer;
	buffer += sizeof(val32);
	boxReadBytes += sizeof(val32);
	box->type = ntohl(val32);

	uint64_t realboxsize;
	box->largesize = 0;
	if (box->size == 0) {
		/* box extends to end of file */
		return -1;
	} else if (box->size == 1) {
		/* large size */
		val32 = *(uint32_t*)buffer;
		buffer += sizeof(val32);
		boxReadBytes += sizeof(val32);
		box->largesize = (uint64_t)ntohl(val32) << 32;

		val32 = *(uint32_t*)buffer;
		buffer += sizeof(val32);
		boxReadBytes += sizeof(val32);
		box->largesize |= (uint64_t)ntohl(val32) & 0xFFFFFFFFULL;
		realboxsize = box->largesize;
	} else {
		realboxsize = box->size;
	}

	return realboxsize;
}

/////////////////////////////////////////////////////
int mp4_demux_parse_ftyp_body(struct mp4_demux *demux,const char* buffer, int maxBytes)
{
	/* major_brand */
	uint32_t val32 = *(uint32_t*)buffer;
	buffer += sizeof(val32);
	int boxReadBytes = sizeof(val32);
	uint32_t majorBrand = ntohl(val32);
	printf("# ftyp: major_brand=%c%c%c%c \n",
		(char)((majorBrand >> 24) & 0xFF),
		(char)((majorBrand >> 16) & 0xFF),
		(char)((majorBrand >> 8) & 0xFF),
		(char)(majorBrand & 0xFF));

	/* minor_version */
	val32 = *(uint32_t*)buffer;
	buffer += sizeof(val32);
	boxReadBytes += sizeof(val32);
	uint32_t minorVersion = ntohl(val32);
	printf("# ftyp: minor_version=%d \n", minorVersion);

	int k = 0;
	while (boxReadBytes + 4 <= maxBytes) {
		/* compatible_brands[] */
		val32 = *(uint32_t*)buffer;
		buffer += sizeof(val32);
		boxReadBytes += sizeof(val32);
		uint32_t compatibleBrands = ntohl(val32);
		printf("# ftyp: compatible_brands[%d]=%c%c%c%c \n", k,
			(char)((compatibleBrands >> 24) & 0xFF),
			(char)((compatibleBrands >> 16) & 0xFF),
			(char)((compatibleBrands >> 8) & 0xFF),
			(char)(compatibleBrands & 0xFF));
		k++;
	}

	return boxReadBytes;
}

std::string strsps;
std::string strpps;
static int mp4_demux_parse_avcc(struct mp4_demux *demux, const char* buffer, int maxBytes)
{
	/* version & profile & level */
	uint32_t val32 = *(uint32_t*)buffer;
	buffer += sizeof(val32);
	int boxReadBytes = sizeof(val32);
	val32 = htonl(val32);
	uint8_t version = (val32 >> 24) & 0xFF;
	uint8_t profile = (val32 >> 16) & 0xFF;
	uint8_t profile_compat = (val32 >> 8) & 0xFF;
	uint8_t level = val32 & 0xFF;
	printf("# avcC: version=%d \n", version);
	printf("# avcC: profile=%d \n", profile);
	printf("# avcC: profile_compat=%d \n", profile_compat);
	printf("# avcC: level=%d \n", level);

	/* length_size & sps_count */
	uint16_t val16 = *(uint16_t*)buffer;
	buffer += sizeof(val16);
	boxReadBytes += sizeof(val16);
	val16 = htons(val16);
	uint8_t lengthSize = ((val16 >> 8) & 0x3) + 1;
	uint8_t spsCount = val16 & 0x1F;
	printf("# avcC: length_size=%d \n", lengthSize);
	printf("# avcC: sps_count=%d \n", spsCount);

	int minBytes = 2 * spsCount;

	int i;
	for (i = 0; i < spsCount; i++) {
		/* sps_length */
		val16 = *(uint16_t*)buffer;
		buffer += sizeof(val16);
		boxReadBytes += sizeof(val16);
		uint16_t spsLength = htons(val16);
		printf("# avcC: sps_length=%d \n", spsLength);

		printf("sps=%d %d %d %d %d %d \n", *buffer, *(buffer+1), *(buffer+2), 
			*(buffer+3), *(buffer+4), *(buffer+5));

		if( i == 0 )
	        strsps.append(buffer, spsLength);            

		minBytes += spsLength;

		buffer += spsLength;
		boxReadBytes += spsLength;
	}

	minBytes++;

	/* pps_count */
	uint8_t ppsCount = *(uint8_t*)buffer;
	buffer += sizeof(ppsCount);
	boxReadBytes += sizeof(ppsCount);
	printf("# avcC: pps_count=%d \n", ppsCount);

	minBytes += 2 * ppsCount;

	for (i = 0; i < ppsCount; i++) {
		/* pps_length */
		val16 = *(uint16_t*)buffer;
		buffer += sizeof(val16);
		boxReadBytes += sizeof(val16);

		uint16_t ppsLength = htons(val16);
		printf("# avcC: pps_length=%d \n", ppsLength);
		printf("pps=%d %d %d %d %d %d \n", *buffer, *(buffer+1), *(buffer+2), 
			*(buffer+3), *(buffer+4), *(buffer+5));

		if( i == 0 )
	        strpps.append(buffer, ppsLength);            

		minBytes += ppsLength;

		buffer += ppsLength;
		boxReadBytes += ppsLength;
	}

	return boxReadBytes;
}

static off_t mp4_demux_parse_stsd(struct mp4_demux *demux,const char* buffer, int maxBytes)
{
	/* version & flags */
	uint32_t val32 = *(uint32_t*)buffer;
	buffer += sizeof(val32);
	int boxReadBytes = sizeof(val32);
	uint32_t flags = ntohl(val32);
	uint8_t version = (flags >> 24) & 0xFF;
	flags &= ((1 << 24) - 1);
	printf("# stsd: version=%d \n", version);
	printf("# stsd: flags=%d \n", flags);

	/* entry_count */
	val32 = *(uint32_t*)buffer;
	buffer += sizeof(val32);
	boxReadBytes += sizeof(val32);
	uint32_t entryCount = ntohl(val32);
	printf("# stsd: entry_count=%d \n", entryCount);

	printf("# stsd: video handler type \n");
	/* size */
	val32 = *(uint32_t*)buffer;
	buffer += sizeof(val32);
	boxReadBytes += sizeof(val32);
	uint32_t size = ntohl(val32);
	printf("# stsd: size=%d \n", size);

	/* type */
	val32 = *(uint32_t*)buffer;
	buffer += sizeof(val32);
	boxReadBytes += sizeof(val32);
	uint32_t type = ntohl(val32);
	printf("# stsd: type=%c%c%c%c \n",
		(char)((type >> 24) & 0xFF),
		(char)((type >> 16) & 0xFF),
		(char)((type >> 8) & 0xFF),
		(char)(type & 0xFF));

	/* reserved */
	val32 = *(uint32_t*)buffer;
	buffer += sizeof(val32);
	boxReadBytes += sizeof(val32);

	/* reserved & data_reference_index */
	val32 = *(uint32_t*)buffer;
	buffer += sizeof(val32);
	boxReadBytes += sizeof(val32);
	uint16_t dataReferenceIndex =
		(uint16_t)(ntohl(val32) & 0xFFFF);
	printf("# stsd: data_reference_index=%d \n",
		dataReferenceIndex);

	int k;
	for (k = 0; k < 4; k++) {
		/* pre_defined & reserved */
		val32 = *(uint32_t*)buffer;
		buffer += sizeof(val32);
		boxReadBytes += sizeof(val32);
	}

	/* width & height */
	val32 = *(uint32_t*)buffer;
	buffer += sizeof(val32);
	boxReadBytes += sizeof(val32);
	printf("# stsd: width=%d \n", (ntohl(val32) >> 16) & 0xFFFF);
	printf("# stsd: height=%d \n", ntohl(val32) & 0xFFFF);

	/* horizresolution */
	val32 = *(uint32_t*)buffer;
	buffer += sizeof(val32);
	boxReadBytes += sizeof(val32);
	float horizresolution = (float)(ntohl(val32)) / 65536.;
	printf("# stsd: horizresolution=%.2f \n",
		horizresolution);

	/* vertresolution */
	val32 = *(uint32_t*)buffer;
	buffer += sizeof(val32);
	boxReadBytes += sizeof(val32);
	float vertresolution = (float)(ntohl(val32)) / 65536.;
	printf("# stsd: vertresolution=%.2f \n",
		vertresolution);

	/* reserved */
	val32 = *(uint32_t*)buffer;
	buffer += sizeof(val32);
	boxReadBytes += sizeof(val32);

	/* frame_count */
	uint16_t val16 = *(uint16_t*)buffer;
	buffer += sizeof(val16);
	boxReadBytes += sizeof(val16);
	uint16_t frameCount = ntohs(val16);
	printf("# stsd: frame_count=%d \n", frameCount);

	/* compressorname */
	char compressorname[32];
	memcpy(compressorname, buffer, 32);
	buffer += sizeof(compressorname);
	boxReadBytes += sizeof(compressorname);			
	boxReadBytes += sizeof(compressorname);
	printf("# stsd: compressorname=%s \n", compressorname);

	/* depth & pre_defined */
	val32 = *(uint32_t*)buffer;
	buffer += sizeof(val32);
	boxReadBytes += sizeof(val32);
	uint16_t depth =
		(uint16_t)((ntohl(val32) >> 16) & 0xFFFF);
	printf("# stsd: depth=%d \n", depth);

	/* codec specific size */
	val32 = *(uint32_t*)buffer;
	buffer += sizeof(val32);
	boxReadBytes += sizeof(val32);
	uint32_t codecSize = ntohl(val32);
	printf("# stsd: codec_size=%d \n", codecSize);

	/* codec specific */
	val32 = *(uint32_t*)buffer;
	buffer += sizeof(val32);
	boxReadBytes += sizeof(val32);
	uint32_t codec = ntohl(val32);
	printf("# stsd: codec=%c%c%c%c \n",
		(char)((codec >> 24) & 0xFF),
		(char)((codec >> 16) & 0xFF),
		(char)((codec >> 8) & 0xFF),
		(char)(codec & 0xFF));

	int nRet = mp4_demux_parse_avcc(demux, buffer, maxBytes - boxReadBytes);
	boxReadBytes += nRet;

	return boxReadBytes;
}
///////////////////////////////////////////////////////////
int mp4_demux_parse_mdat(struct mp4_demux *demux,const char* buffer, int maxBytes)
{
	std::fstream fs264;
	fs264.open("./222.264", std::ios::binary | std::ios::out);
	char startcode[] = {0,0,0,1};
	int boxReadBytes = 0;
    fs264.write(strsps.data(), strsps.size());
    fs264.write(strpps.data(), strpps.size());
	while( boxReadBytes < maxBytes )
	{
		uint32_t val32 = *(uint32_t*)buffer;
		buffer += sizeof(val32);
		boxReadBytes += sizeof(val32);

		uint32_t framelen = ntohl(val32);
//		printf("framelen %d %d \n", framelen, *buffer&0x1f);
        fs264.write(startcode, sizeof(startcode));
        fs264.write(buffer, framelen);
		buffer += framelen;
		boxReadBytes += framelen;
	}	

    fs264.close();
	return 0;
}

int mp4_demux_parse_moov(struct mp4_demux *demux,const char* buffer, int maxBytes)
{
	int leftlen = 0;
	while( 1 )
	{
		mp4_box_header_t header;
		int boxlen = getonebox(buffer, maxBytes, &header);
		if( boxlen < 0 )
		{
			return -1;
		}
		int headerlen = header.largesize > 0?16:8;

		char type[] = {(char)(header.type>>24), (char)(header.type>>16), 
			(char)(header.type>>8), (char)header.type};
		std::string boxtype;
		boxtype.append(type, 4);
		printf("boxtype %s boxsize %d\n", boxtype.c_str(), boxlen);
		if( boxtype == "trak" || 
			boxtype == "mdia" ||
			boxtype == "stbl" ||
//			boxtype == "stsd" ||
			boxtype == "dinf" ||
//			boxtype == "dref" ||
			boxtype == "minf")
		{
			buffer += headerlen;
			leftlen += headerlen;
//			mp4_demux_parse_trak(demux, buffer+headerlen, boxlen - headerlen);					
		}
		else if( boxtype == "stsd" )
		{
			int nRet = mp4_demux_parse_stsd(demux, buffer+headerlen, maxBytes-headerlen);
			printf("mp4_demux_parse_stsd %d \n", nRet);

			buffer += boxlen;
			leftlen += boxlen;			
		}
		else 
		{
			buffer += boxlen;
			leftlen += boxlen;			
		}

		if( leftlen >= maxBytes )
			break;
	}
}

int mp4_demux_parse(struct mp4_demux *demux,const char *boxtype, const char* buffer, int maxBytes)
{
//	printf("mp4_demux_parse %s \n", boxtype.c_str());
	if( strcmp(boxtype, "ftyp") == 0 )	
	{
		mp4_demux_parse_ftyp_body(demux, buffer, maxBytes);
	}
	else if( strcmp(boxtype, "free") == 0 )	
	{
		
	}
	else if( strcmp(boxtype, "mdat") == 0 )
	{
		mp4_demux_parse_mdat(demux, buffer, maxBytes);		
	}
	else if( strcmp(boxtype, "moov") == 0 )	
	{
		mp4_demux_parse_moov(demux, buffer, maxBytes);				
	}

	return 0;
}
