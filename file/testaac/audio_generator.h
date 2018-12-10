#ifndef __AUDIO_GENERATOR_H__
#define __AUDIO_GENERATOR_H__

void *audio_generator_alloc(int sample_rate, int bits_per_sample, int channels, int nb_samples);

int audio_generator_get_audio_frame(void* handle, const char** frame, int *length);

int audio_generator_destroy(void *handle);

#endif