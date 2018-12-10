#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string>

#include "typedefs.h"
#include "mp4_demux.h"
#include "demux.h"

char tempbuffer[20 * 1024 * 1024 ] = {0};
std::string strmdat;
int mp4_demux_listboxes(void *handle)
{
	struct mp4_demux *demux= (struct mp4_demux *)handle;
	int parentReadBytes = 0;
	int ret = 0;

	while ( !feof(demux->file) )
	{
		uint32_t val32;
		struct mp4_box_header_t box;
		memset(&box, 0, sizeof(box));

		/* box size */
		size_t _count = fread(&val32, sizeof(val32), 1, demux->file);
		int boxReadBytes = _count;
		box.size = ntohl(val32);

		/* box type */
		_count = fread(&val32, sizeof(val32), 1, demux->file);
		boxReadBytes += _count;
		box.type = ntohl(val32);

		printf("offset 0x%lX box '%c%c%c%c' boxsize %d \n",
			ftello(demux->file),
			(box.type >> 24) & 0xFF,
			(box.type >> 16) & 0xFF,
			(box.type >> 8) & 0xFF,
			box.type & 0xFF, box.size);

		int boxbodySize;
		if (box.size == 0) {
			/* box extends to end of file */
			break;
		} else if (box.size == 1) {
			/* large size */
			_count = fread(&val32, sizeof(val32), 1, demux->file);
			boxReadBytes += _count;
			box.largesize = (uint64_t)ntohl(val32) << 32;

			_count = fread(&val32, sizeof(val32), 1, demux->file);
			boxReadBytes += _count;
			box.largesize |= (uint64_t)ntohl(val32) & 0xFFFFFFFFULL;
			boxbodySize = box.largesize;
			boxbodySize -= 16;
		} else {
			boxbodySize = box.size;
			boxbodySize -= 8;
		}

		/* box extended type */
		size_t count = fread(tempbuffer, boxbodySize, 1, demux->file);
		if( count <= 0 )
			break;

		char type[] = {(char)(box.type>>24), (char)(box.type>>16), 
			(char)(box.type>>8), (char)box.type};
		std::string boxtype;
		boxtype.append(type, 4);
		if( boxtype != "mdat" )
			mp4_demux_parse(demux, boxtype.data(), tempbuffer, boxbodySize);
		else
		{
			strmdat.append(tempbuffer, boxbodySize);
		}
	}
	mp4_demux_parse(demux, "mdat", strmdat.data(), strmdat.size());

	return 0;
}

void* mp4_demux_open2(const char *filename)
{
	int err = 0, ret;

	struct mp4_demux *demux = (struct mp4_demux *)malloc(sizeof(*demux));
	if (demux == NULL) {
		printf("allocation failed \n");
		return NULL;
	}
	memset(demux, 0, sizeof(*demux));

	FILE *file = fopen(filename, "rb");
	if (file == NULL) {
		printf("failed to open file '%s' \n", filename);
		free(demux);
		return NULL;
	}

	demux->file = file;
	return (void*)demux;
}

int mp4_demux_close(void* handle)
{
	struct mp4_demux *demux= (struct mp4_demux *)handle;
	if (demux) {
		if (demux->file)
			fclose(demux->file);
	}

	free(demux);

	return 0;
}

