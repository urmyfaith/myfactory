#ifndef _MP4_DEMUX_H_
#define _MP4_DEMUX_H_

#ifdef __cplusplus
extern "C"  {
#endif /* __cplusplus */

void* mp4_demux_open2(const char *filename);

int mp4_demux_close(void* handle);

int mp4_demux_listboxes(void *handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_LIBMP4_H_ */
