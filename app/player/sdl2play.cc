#include <thread>

#include <stdio.h>
#include <unistd.h>

#include "SDL2/SDL.h"
#include "sdl2play.h"

struct SDL2Desc_t 
{
    SDL_Window *screen;   
    SDL_Renderer* sdlRenderer;  
    SDL_Texture* sdlTexture;  
    SDL_Rect sdlRect; 
	SDL_AudioDeviceID audio_dev;
};

int sdl_pollevent(SDL2Desc_t *inst)
{
	SDL_Event event;
	SDL_PollEvent(&event);

	if (event.type == SDL_KEYDOWN) 
	{
		switch (event.key.keysym.sym) 
		{
		case SDLK_f:
			/* press key 'f' to toggle fullscreen */			
/*
			if (flags&SDL_WINDOW_FULLSCREEN_DESKTOP)
				flags &=
					~SDL_WINDOW_FULLSCREEN_DESKTOP;
			else
				flags |=
					SDL_WINDOW_FULLSCREEN_DESKTOP;

			SDL_SetWindowFullscreen(inst->screen, flags);
*/
			break;

		default:
			break;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////
void* sdlplay_open(void* parenthwnd)
{
	//Èç¹ûÊ¹ÓÃ±¾µØ´°¿Ú£¬ÐèÒªÉèÖÃ´°¿Ú¾ä±ú
	if( parenthwnd )
	{
		char variable[256]; 
		sprintf(variable,"0x%1x",parenthwnd); // ¸ñÊ½»¯×Ö·û´® 
		int ret = SDL_setenv("SDL_WINDOWID", variable, 1);  
		if( ret != 0 )
			return NULL;
	}

//    int flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO;// | SDL_INIT_TIMER;
    int flags = SDL_INIT_EVERYTHING;
	int iReturn=SDL_Init(flags);
	if( iReturn != 0 )
		return NULL;

	SDL2Desc_t *inst = new SDL2Desc_t;
	inst->screen = NULL;

	return inst;
}

int sdlplay_pollevent(void *handle)
{
	SDL2Desc_t *inst = (SDL2Desc_t*)handle;
	return sdl_pollevent(inst);
}

int sdlplay_set_video(void* handle, const char* windowtitle, int iWidth,int iHeight)
{
	SDL2Desc_t *inst = (SDL2Desc_t*)handle;
	printf("SDL_InitializeVideo %d %d \n", iWidth, iHeight);

    //SDL 2.0 Support for multiple windows  
    inst->screen = SDL_CreateWindow(windowtitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,  
        iWidth, iHeight, SDL_WINDOW_RESIZABLE);//SDL_WINDOW_OPENGL);    
    if(!inst->screen) {    
        printf("SDL: could not create window - exiting:%s\n",SDL_GetError());    
        return -1;  
    }		

    inst->sdlRenderer = SDL_CreateRenderer(inst->screen, -1, 0);    
    //IYUV: Y + U + V  (3 planes)  
    //YV12: Y + V + U  (3 planes)  
    inst->sdlTexture = SDL_CreateTexture(inst->sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, iWidth, iHeight);    
  
    inst->sdlRect.x=0;  
    inst->sdlRect.y=0;  
    inst->sdlRect.w=iWidth;  
    inst->sdlRect.h=iHeight;  

	return 0;
}

int sdlplay_set_audio(void* handle, int iSampleRate,int iChannels,void *pUserData, sdlplay_audio_callback callback)
{
	SDL2Desc_t *inst = (SDL2Desc_t*)handle;
	printf("SDL_InitializeAudio %d %d \n",iSampleRate, iChannels);

	SDL_AudioSpec wanted_spec, spec;  
	wanted_spec.freq = iSampleRate;  
	wanted_spec.channels = iChannels;  
	wanted_spec.samples = 1024;
	wanted_spec.userdata = pUserData;  
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.silence = 0;  
	wanted_spec.callback = callback;  
	inst->audio_dev = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
	if( !inst->audio_dev )  
	{  
		printf("SDL_OpenAudio: %s/n", SDL_GetError());  
		return -1;  
	}  
    SDL_PauseAudioDevice(inst->audio_dev, 0);

	return 0;
}

int sdlplay_display_yuv(void* handle, uint8_t* data[], int linesize[])
{
	SDL2Desc_t *inst = (SDL2Desc_t*)handle;
	sdl_pollevent(inst);

    SDL_UpdateYUVTexture(inst->sdlTexture, &inst->sdlRect, data[0], linesize[0], 
	    data[1], linesize[1], data[2], linesize[2]);  
      
    SDL_RenderClear( inst->sdlRenderer );    
    SDL_RenderCopy( inst->sdlRenderer, inst->sdlTexture,  NULL, &inst->sdlRect);    
    SDL_RenderPresent( inst->sdlRenderer );    

    return 0;
}

int sdlplay_display(void* handle, uint8_t* data[], int linesize[])
{
	SDL2Desc_t *inst = (SDL2Desc_t*)handle;
	sdl_pollevent(inst);

    SDL_UpdateTexture(inst->sdlTexture, &inst->sdlRect, data, linesize[0]);
      
    SDL_RenderClear( inst->sdlRenderer );    
    SDL_RenderCopy( inst->sdlRenderer, inst->sdlTexture,  NULL, &inst->sdlRect);    
    SDL_RenderPresent( inst->sdlRenderer );    

    return 0;
}

int sdlplay_close(void* handle)
{
	SDL2Desc_t *inst = (SDL2Desc_t*)handle;

    SDL_Quit();  
	delete inst;  

	return 0;	
}

////////////////////////////////////////////////////
int sdlplay_mutex_alloc(void **mutex)
{
	SDL_mutex *wait_mutex = SDL_CreateMutex();
	if( !wait_mutex )
	{
		printf("SDL_CreateMutex error: %s\n", SDL_GetError());
		return -1;
	}

	*mutex = wait_mutex;
    return 0;
}
int sdlplay_mutex_lock(void *mutex)
{
	if( !mutex)
		return -1;

	SDL_mutex *wait_mutex = (SDL_mutex*)mutex;

    SDL_LockMutex(wait_mutex);
    return 0;
}
int sdlplay_mutex_unlock(void *mutex)
{
	if( !mutex)
		return -1;
	
	SDL_mutex *wait_mutex = (SDL_mutex*)mutex;

    SDL_UnlockMutex(wait_mutex);
    return 0;
}
int sdlplay_mutex_free(void *mutex)
{
	if( !mutex)
		return -1;
	
	SDL_mutex *wait_mutex = (SDL_mutex*)mutex;

    SDL_DestroyMutex(wait_mutex);
    return 0;
}
////////////////////////////////////////
int sdlplay_cond_alloc(void **cond)
{
	SDL_cond *wait_cond = SDL_CreateCond();
	if( !wait_cond )
	{
		printf("SDL_CreateCond error: %s\n", SDL_GetError());
		return -1;
	}

	*cond = wait_cond;
    return 0;
}
int sdlplay_cond_wait(void *cond, void* mutex)
{
	if( !cond)
		return -1;

	SDL_cond *wait_cond = (SDL_cond*)cond;
	SDL_mutex *wait_mutex = (SDL_mutex*)mutex;

    SDL_CondWait(wait_cond, wait_mutex);
    return 0;
}
int sdlplay_cond_wait_timeout(void *cond, void* mutex, uint32_t milliseconds)
{
	if( !cond)
		return -1;

	SDL_cond *wait_cond = (SDL_cond*)cond;
	SDL_mutex *wait_mutex = (SDL_mutex*)mutex;

    SDL_CondWaitTimeout(wait_cond, wait_mutex, milliseconds);
    return 0;
}
int sdlplay_cond_signal(void *cond)
{
	if( !cond)
		return -1;
	
	SDL_cond *wait_cond = (SDL_cond*)cond;

    SDL_CondSignal(wait_cond);
    return 0;
}
int sdlplay_cond_free(void *cond)
{
	if( !cond)
		return -1;
	
	SDL_cond *wait_cond = (SDL_cond*)cond;

    SDL_DestroyCond(wait_cond);
    return 0;
}
////////////////////////////////////////
int sdlplay_sem_alloc(void **sem, uint32_t initial_value)
{
	SDL_sem *wait_sem = SDL_CreateSemaphore(initial_value);
	if( !wait_sem )
	{
		printf("SDL_CreateSemaphore error: %s\n", SDL_GetError());
		return -1;
	}

	*sem = wait_sem;
    return 0;
}
int sdlplay_sem_wait(void *sem)
{
	if( !sem)
		return -1;

	SDL_sem *wait_sem = (SDL_sem*)sem;

    SDL_SemWait(wait_sem);
    return 0;
}
int sdlplay_sem_wait_timeout(void *sem, uint32_t milliseconds)
{
	if( !sem)
		return -1;

	SDL_sem *wait_sem = (SDL_sem*)sem;

    SDL_SemWaitTimeout(wait_sem, milliseconds);
    return 0;
}
int sdlplay_sem_post(void *sem)
{
	if( !sem)
		return -1;
	
	SDL_sem *wait_sem = (SDL_sem*)sem;

    SDL_SemPost(wait_sem);
    return 0;
}
int sdlplay_sem_free(void *sem)
{
	if( !sem)
		return -1;
	
	SDL_sem *wait_sem = (SDL_sem*)sem;

    SDL_DestroySemaphore(wait_sem);
    return 0;
}