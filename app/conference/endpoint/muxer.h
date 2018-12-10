#ifndef __MUXER_H__
#define __MUXER_H__

void* muxer_alloc(const char* sourcetype);

int muxer_setframe(void* handle, const char* h264frame, int framelength);

int muxer_getpacket(void* handle, const char** rtp_buffer, 
	int *rtp_packet_length, int *last_rtp_packet_length, int *rtp_packet_count);

int muxer_free(void *handle);

#endif