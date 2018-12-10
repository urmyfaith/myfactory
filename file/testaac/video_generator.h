#ifndef __VIDEO_GENERATOR_H__
#define __VIDEO_GENERATOR_H__

void *video_generator_alloc(int width, int height, int pixel_format);

int video_generator_get_yuv420p_frame(void* handle, const char** frame, int *length);

int video_generator_destroy(void *handle);

#endif