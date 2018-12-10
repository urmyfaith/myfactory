#ifndef __DEMUX_H__
#define __DEMUX_H__

#ifdef __cplusplus
extern "C"  {
#endif /* __cplusplus */

int mp4_demux_parse(struct mp4_demux *demux,const char *boxtype, const char* buffer, int maxBytes);

int mp4_demux_parse_ftyp(struct mp4_demux *demux,const char* buffer, int maxBytes);

int mp4_demux_parse_mdat(struct mp4_demux *demux,const char* buffer, int maxBytes);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_LIBMP4_H_ */
