/*
A simple sound library for CSE 20211 by Douglas Thain (dthain@nd.edu).
This work is made available under the Creative Commons Attribution license.
https://creativecommons.org/licenses/by/4.0/
For course assignments, you should not change this file.
For complete documentation, see:
http://www.nd.edu/~dthain/courses/cse20211/fall2013/wavfile
*/

#include "wavfile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct wavfile_header {
	char	riff_tag[4];
	int	riff_length;
	char	wave_tag[4];
	char	fmt_tag[4];
	int	fmt_length;
	short	audio_format;
	short	num_channels;
	int	sample_rate;
	int	byte_rate;
	short	block_align;
	short	bits_per_sample;
	char	data_tag[4];
	int	data_length;
};

typedef struct {
	struct wavfile_header header;
	FILE *file;
}WAVFileDesc_t;

void* wavfile_open( const char *filename, int audioformat, int samples_per_second, int bits_per_sample, int num_channels)
{
	struct wavfile_header header;

	strncpy(header.riff_tag,"RIFF",4);
	strncpy(header.wave_tag,"WAVE",4);
	strncpy(header.fmt_tag,"fmt ",4);
	strncpy(header.data_tag,"data",4);

	header.riff_length = 0;
	header.fmt_length = 16;
	header.audio_format = audioformat;
	header.num_channels = num_channels;
	header.sample_rate = samples_per_second;
	header.byte_rate = samples_per_second*(bits_per_sample/8);
	header.block_align = bits_per_sample/8;
	header.bits_per_sample = bits_per_sample;
	header.data_length = 0;

	FILE * file = fopen(filename,"w+");
	if(!file) return NULL;

	fwrite(&header,sizeof(header),1,file);
	fflush(file);

	WAVFileDesc_t *desc = new WAVFileDesc_t;
	desc->file = file;
	desc->header = header;

	return (void*)desc;
}

int wavfile_write(void *handle, const char*data, int length )
{
	WAVFileDesc_t *desc = (WAVFileDesc_t*)handle;
	return fwrite(data, 1, length, desc->file);
}

void wavfile_close(void *handle)
{
	WAVFileDesc_t *desc = (WAVFileDesc_t*)handle;
	int file_length = ftell(desc->file);

	int data_length = file_length - sizeof(struct wavfile_header);
	fseek(desc->file,sizeof(struct wavfile_header) - sizeof(int),SEEK_SET);
	fwrite(&data_length,sizeof(data_length),1,desc->file);

	int riff_length = file_length - 8;
	fseek(desc->file,4,SEEK_SET);
	fwrite(&riff_length,sizeof(riff_length),1,desc->file);

	fclose(desc->file);

	delete desc;

}