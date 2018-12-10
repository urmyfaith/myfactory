#ifndef __RESAMPLE_H__
#define __RESAMPLE_H__

struct sound_resampled
{
	uint8_t *data;
	int linesize;
};

void* resample_open(int in_channels,int in_sample_fmt,int in_sample_rate);

int resample_sound(void* handle, uint8_t *data, int linesize, int samplecount, sound_resampled *frame);

int resample_close(void* handle);

#endif