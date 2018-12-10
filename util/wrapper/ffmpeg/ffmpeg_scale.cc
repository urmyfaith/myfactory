#ifdef __cplusplus
extern "C"{
#endif
	#include "libavcodec/avcodec.h"  
	#include "libavformat/avformat.h"  
	#include "libswscale/swscale.h"  
	#include "libavutil/imgutils.h"  
#ifdef __cplusplus
}
#endif

#include "ffmpeg_scale.h"

struct scaledesc_t
{
	AVFrame *pFrameYUV;
	unsigned char* out_buffer;	
	SwsContext *img_convert_ctx;
	int width;
	int height;
};

void* scale_open(int width, int height, int pix_fmt)
{
	scaledesc_t * inst = new scaledesc_t;
	if( !inst )
		return NULL;

	inst->width = width;
	inst->height = height;
    inst->pFrameYUV=av_frame_alloc();  
    inst->out_buffer=(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,  width, height,1));  
    av_image_fill_arrays(inst->pFrameYUV->data, inst->pFrameYUV->linesize, inst->out_buffer,  
        AV_PIX_FMT_YUV420P,width, height,1);  

    inst->img_convert_ctx = sws_getContext(width, height, (AVPixelFormat)pix_fmt,   
        width, height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);   

	return inst;
}

int scale_image(void* handle, uint8_t **data, int linesize[], image_scaled *image)
{
	scaledesc_t * inst = (scaledesc_t*)handle;

    sws_scale(inst->img_convert_ctx, data, linesize, 0, inst->height,   
        inst->pFrameYUV->data, inst->pFrameYUV->linesize);  

    image->data = inst->pFrameYUV->data;
    image->linesize = inst->pFrameYUV->linesize;

    return 0;
}

int scale_close(void* handle)
{
	scaledesc_t * inst = (scaledesc_t*)handle;
	av_frame_free(&inst->pFrameYUV);
	av_free(inst->out_buffer);

	delete inst;

	return 0;
}
