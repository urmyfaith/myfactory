#include "audio_generator.h"
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

struct audio_generator_tag_t
{
	std::string audio_frame;

    float t, tincr, tincr2;
	int bits_per_sample;
	int channels;
	int nb_samples;
	int sample_rate;
};

void *audio_generator_alloc(int sample_rate, int bits_per_sample, int channels, int nb_samples)
{
	audio_generator_tag_t *inst = new audio_generator_tag_t;

    /* init signal generator */
    inst->t     = 0;
    inst->tincr = 2 * M_PI * 110.0 / sample_rate;
    /* increment frequency by 110 Hz per second */
    inst->tincr2 = 2 * M_PI * 110.0 / sample_rate / sample_rate;

	inst->bits_per_sample = bits_per_sample;
	inst->channels = channels;
	inst->nb_samples = nb_samples;
	inst->sample_rate = sample_rate;

	return inst;
}

int audio_generator_get_audio_frame(void* handle, const char** frame, int *length)
{
	audio_generator_tag_t *inst = (audio_generator_tag_t*)handle;

	inst->audio_frame.clear();
    int j, i;
    for (j = 0; j < inst->nb_samples; j++) 
    {
	    int v = (int)(sin(inst->t) * 10000);
	    for (i = 0; i < inst->channels; i++)
	    {
	    	inst->audio_frame.append((const char*)&v, inst->bits_per_sample/8);
	    }

	    inst->t     += inst->tincr;
	    inst->tincr += inst->tincr2;
    }
    *frame = inst->audio_frame.data();
    *length = inst->audio_frame.size();

	return 0;
}

int audio_generator_destroy(void *handle)
{
	audio_generator_tag_t *inst = (audio_generator_tag_t*)handle;
	
	delete inst;

	return 0;	
}
