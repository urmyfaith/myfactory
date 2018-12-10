#include <string>

int url_getmsg(std::string &buffer, std::string &msg);

void* rtsp_demux_init(const char* sessionid, int local_rtp_port);

int rtsp_demux_parse(void* handle, const char* request, const char** response, int &transport_proto, int &canSend);

int rtsp_demux_getfileext(void* handle, const char** fileext);

int rtsp_demux_getfilename(void* handle, const char** filename);

int rtsp_demux_get_client_rtp_port(void* handle);

int rtsp_demux_close(void* handle);