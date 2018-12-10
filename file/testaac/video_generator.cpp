#include "video_generator.h"
#include <string>
#include <stdio.h>

struct video_generator_tag_t
{
	std::string video_frame;
	std::string frameY;
	std::string frameU;
	std::string frameV;
    int linesizeY;
    int linesizeU;
    int linesizeV;

    int frame_index;
    int width;
    int height;
    int pixel_format;
};

void *video_generator_alloc(int width, int height, int pixel_format)
{
	video_generator_tag_t *inst = new video_generator_tag_t;

	inst->frame_index = 0;
//	inst->width = (width/4 + width/4%2) * 4;
//	inst->height = (height/4 + height/4%2) * 4;
	inst->width = ( width / 4 ) * 4;
	inst->height = ( height / 4 ) * 4;
	inst->pixel_format = pixel_format;

	inst->linesizeY = inst->width;
	inst->linesizeU = inst->linesizeY / 2;
	inst->linesizeV = inst->linesizeY / 2;

	inst->frameY.resize(inst->width * inst->height);
	inst->frameU.resize(inst->width * inst->height / 4);
	inst->frameV.resize(inst->width * inst->height / 4);
	inst->video_frame.reserve(inst->width * inst->height * 3 / 2);
	return inst;
}

int video_generator_get_yuv420p_frame(void* handle, const char** frame, int *length)
{
	video_generator_tag_t *inst = (video_generator_tag_t*)handle;

	char *dataY = (char*)inst->frameY.data();
    int x, y, i = inst->frame_index;
    for (y = 0; y < inst->height; y++)
        for (x = 0; x < inst->width; x++)
        {
//			printf("position %u %u %u\n",inst->width * inst->height, y * inst->linesizeY + x, inst->frameY.capacity());
            dataY[y * inst->linesizeY + x] = x + y + i * 3;
        }

    /* Cb and Cr */
	char *dataU = (char*)inst->frameU.data();
	char *dataV = (char*)inst->frameV.data();
    for (y = 0; y < inst->height / 2; y++) {
        for (x = 0; x < inst->width / 2; x++) {
            dataU[y * inst->linesizeU + x] = 128 + y + i * 2;
            dataV[y * inst->linesizeV + x] = 64 + x + i * 5;
        }
    }

    inst->frame_index++;
	inst->video_frame.clear();
	inst->video_frame.append(inst->frameY.data(), inst->frameY.size());
	inst->video_frame.append(inst->frameU.data(), inst->frameU.size());
	inst->video_frame.append(inst->frameV.data(), inst->frameV.size());
    *frame = inst->video_frame.data();
    *length = inst->video_frame.size();
    
	return 0;
}

int video_generator_destroy(void *handle)
{
	video_generator_tag_t *inst = (video_generator_tag_t*)handle;
	
	delete inst;
	return 0;	
}
