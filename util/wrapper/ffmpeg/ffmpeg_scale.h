#ifndef __FFMPEG_SCALE_H__
#define __FFMPEG_SCALE_H__

struct image_scaled
{
	uint8_t **data;
	int *linesize;
};

void* scale_open(int width, int height, int pix_fmt);

int scale_image(void* handle, uint8_t **data, int linesize[], image_scaled *image);

int scale_close(void* handle);

#endif