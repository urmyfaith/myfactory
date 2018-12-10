/*
A simple sound library for CSE 20211 by Douglas Thain.
This work is made available under the Creative Commons Attribution license.
https://creativecommons.org/licenses/by/4.0/
For course assignments, you should not change this file.
For complete documentation, see:
http://www.nd.edu/~dthain/courses/cse20211/fall2013/wavfile
*/

#ifndef _WAVFILE_H__
#define _WAVFILE_H__

#include <stdio.h>
#include <inttypes.h>

#define WAV_AUDIOFORMAT_PCM 1
#define WAV_AUDIOFORMAT_ALAW 6
#define WAV_AUDIOFORMAT_ULAW 7

void* wavfile_open( const char *filename, int audioformat, int samples_per_second, int bits_per_sample, int num_channels);

int wavfile_write(void *handle, const char*data, int length );

void wavfile_close(void *handle);

#endif
