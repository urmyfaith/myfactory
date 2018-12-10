#ifndef __SDLPLAY_H__
#define __SDLPLAY_H__

typedef void (*sdlplay_audio_callback)(void *userdata, uint8_t *stream, int len);

void* sdlplay_open(void* parenthwnd);
int sdlplay_pollevent(void *handle);

int sdlplay_set_video(void* handle, const char* windowtitle, int iWidth,int iHeight );

int sdlplay_set_audio(void* handle, int iSampleRate,int iChannels,void *pUserData, sdlplay_audio_callback callback);

int sdlplay_display_yuv(void* handle, uint8_t* data[], int linesize[]);
int sdlplay_display(void* handle, uint8_t* data[], int linesize[]);

int sdlplay_close(void* handle);

//////////////////////////////////
int sdlplay_mutex_alloc(void **mutex);
int sdlplay_mutex_lock(void *mutex);
int sdlplay_mutex_unlock(void *mutex);
int sdlplay_mutex_free(void *mutex);

int sdlplay_cond_alloc(void **cond);
int sdlplay_cond_wait(void *cond, void* mutex);
int sdlplay_cond_wait_timeout(void *cond, void* mutex, uint32_t milliseconds);
int sdlplay_cond_signal(void *cond);
int sdlplay_cond_free(void *cond);

int sdlplay_sem_alloc(void **sem, uint32_t initial_value);
int sdlplay_sem_wait(void *sem);
int sdlplay_sem_wait_timeout(void *sem, uint32_t milliseconds);
int sdlplay_sem_post(void *sem);
int sdlplay_sem_free(void *sem);

#endif