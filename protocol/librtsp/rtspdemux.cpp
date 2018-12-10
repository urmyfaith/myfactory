#include <sstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <string>
#include "rtspdemux.h"

typedef struct 
{
    std::string client_ip;
    int client_rtp_port;
    int server_rtp_port;
    std::string client_session_id;
    int transport_protocol;//0-udp,1-tcp,2-http

    std::string filename;
    std::string filenameext;
    std::string response;
}rtsp_demux_desc_t;

int url_getmsg(std::string &buffer, std::string &msg)
{
    int pos = buffer.find("\r\n\r\n");
    if( pos < 0 )
        return -1;

    msg = buffer.substr(0,pos+4);
    buffer.erase(0,pos+4);
    return 0;
}
/////////////////////////////////////////////
std::string getResponse_OPTIONS(int errorCode, const char *server, std::string &seq)
{
    std::string strResponse;
    if( errorCode == 200 )
        strResponse = "RTSP/1.0 200 OK \r\n" \
                        "Cseq: " + seq + " \r\n" \
                        "Public: DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, OPTIONS, ANNOUNCE, RECORD\r\n" \
                        "\r\n";
    else
        strResponse = "RTSP/1.0 404 Not Found \r\n" \
                        "Cseq: " + seq + " \r\n" \
                        "Public: DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, OPTIONS, ANNOUNCE, RECORD\r\n" \
                        "\r\n";

    return strResponse;
}

std::string getResponse_DESCRIBE(std::string &streamtype, std::string &seq)
{
    std::string headers = "RTSP/1.0 200 OK\r\n" \
                        "Cseq: " + seq + " \r\n" \
                        "Content-Type: application/sdp \r\n";
    
    std::string strSDP;
    if( streamtype == "264" )    
        strSDP = "v=0\r\n" \
                "m=video 0 RTP/AVP 96\r\n" \
                "a=rtpmap:96 H264/90000\r\n" \
    //            "fmtp:96 packetization-mode=1;profile-level-id=64000D;sprop-parameter-sets=Z2QADaw06BQfoQAAAwABAAADADKPFCqg,aO4BLyw=\r\n" \

                "c=IN IP4 0.0.0.0";
    else if( streamtype == "aac" )    
        strSDP = "v=0\r\n" \
                "m=audio  0 RTP/AVP 97\r\n" \
                "a=rtpmap:97 mpeg4-generic/24000/2\r\n" \
                "a=fmtp:97 config=1310;mode=AAC-hbr; SizeLength=13; IndexLength=3;IndexDeltaLength=3" \
//                "a=fmtp:97 profile-level-id=15; config=1310;streamtype=5; ObjectType=64; mode=AAC-hbr; SizeLength=13; IndexLength=3;IndexDeltaLength=3" \

                "c=IN IP4 0.0.0.0";
    else if( streamtype == "alaw" )    
        strSDP = "v=0\r\n" \
                "m=audio  0 RTP/AVP 8\r\n" \
                "a=rtpmap:8 pcma/44100/2\r\n" \
                "a=framerate:25\r\n" \
                "c=IN IP4 0.0.0.0";
    else if( streamtype == "mulaw" )    
        strSDP = "v=0\r\n" \
                "m=audio  0 RTP/AVP 0\r\n" \
                "a=rtpmap:0 pcmu/44100/2\r\n" \
                "a=framerate:25\r\n" \
                "c=IN IP4 0.0.0.0";

    strSDP += "\r\n";

    headers += "Content-length: ";
    std::stringstream ss;
    ss<<strSDP.size();
    headers += ss.str();
    headers += "\r\n\r\n";

    std::string strResponse = headers + strSDP;
    return strResponse;
}

std::string getResponse_SETUP_UDP(const char *sessionID, int clientPort, int serverPort, std::string &seq)
{
    std::string headers = "RTSP/1.0 200 OK \r\n" \
                    "Cseq: " + seq + " \r\n";
    
    std::string strSession = "Session: ";
    strSession += sessionID;
    strSession += ";timeout=80 \r\n";

    std::string strTransport = "Transport: RTP/AVP;unicast;client_port=";
    std::stringstream ss;
    
    ss<<clientPort<<"-"<<clientPort+1;
    strTransport += ss.str();
    strTransport += ";server_port=";
    ss.str("");
    ss<<serverPort<<"-"<<serverPort+1;
    strTransport += ss.str() + "\r\n";

    std::string strResponse = headers + strSession;
    strResponse += strTransport;
    strResponse += "\r\n";

    return strResponse;
}

std::string getResponse_SETUP_TCP(const char *sessionID, std::string &seq)
{
    std::string headers = "RTSP/1.0 200 OK \r\n" \
                     "Cseq: " + seq + " \r\n";
   
    std::string strSession = "Session: ";
    strSession += sessionID;
    strSession += ";timeout=80 \r\n";

    std::string strTransport = "Transport: RTP/AVP/TCP;unicast;interleaved=0-1;mode=play \r\n";

    std::string strResponse = headers + strSession;
    strResponse += strTransport;
    strResponse += "\r\n";

    return strResponse;
}

std::string getResponse_PLAY(const char *sessionID, std::string &seq)
{
    std::string strResponse = "RTSP/1.0 200 OK \r\n" \
                    "Cseq: " + seq + " \r\n";

    std::string strSession = "Session: ";
    strSession += sessionID;
    strSession += ";timeout=80 \r\n";
    strSession += "\r\n";
    
    strResponse += strSession;

    return strResponse;
}   

std::string getResponse_PAUSE(const char *sessionID, std::string &seq)
{
    std::string strResponse = "RTSP/1.0 200 OK \r\n" \
                    "Cseq: " + seq + " \r\n";

    std::string strSession = "Session: ";
    strSession += sessionID;
    strSession += "\r\n";
    strSession += "\r\n";
    
    strResponse += strSession;

    return strResponse;
}

std::string getResponse_TEARDOWN(const char *sessionID, std::string &seq)
{
    std::string strResponse = "RTSP/1.0 200 OK \r\n" \
                    "Cseq: " + seq + " \r\n";

    std::string strSession = "Session: ";
    strSession += sessionID;
    strSession += "\r\n";
    strSession += "\r\n";
    
    strResponse += strSession;

    return strResponse;
}

////////////////////////////////////////////
void* rtsp_demux_init(const char* sessionid, int local_rtp_port)
{
    rtsp_demux_desc_t* handle = new rtsp_demux_desc_t;
    handle->client_session_id = sessionid;
    handle->server_rtp_port = local_rtp_port;

    return (void*)handle;
}

int rtsp_demux_getfileext(void* handle, const char** fileext)
{
    rtsp_demux_desc_t* rtsp_demux = (rtsp_demux_desc_t*)handle;
    *fileext = rtsp_demux->filenameext.c_str();

    return 0;
}

int rtsp_demux_getfilename(void* handle, const char** filename)
{
    rtsp_demux_desc_t* rtsp_demux = (rtsp_demux_desc_t*)handle;
    *filename = rtsp_demux->filename.c_str();

    return 0;    
}

int rtsp_demux_parse(void* handle, const char* request, const char** response, int &transport_proto, int &canSend)
{
    rtsp_demux_desc_t* rtsp_demux = (rtsp_demux_desc_t*)handle;

    canSend = 0;
    std::string strRequest = request;
    int pos = strRequest.find(' ');
    std::string method = strRequest.substr(0,pos), strResponse;

    if( rtsp_demux->filename == "" )
    {
        int pos1 = strRequest.find(' ', pos+1);
        std::string url = strRequest.substr(pos+1,pos1-pos-1);
        pos = url.rfind('/');
        rtsp_demux->filename = url.substr(pos+1);
        pos = rtsp_demux->filename.rfind('.');
        rtsp_demux->filenameext = rtsp_demux->filename.substr(pos+1);    

//        printf("filename=%s filenameext=%s\n", rtsp_demux->filename.c_str(), rtsp_demux->filenameext.c_str());            
    }

    pos = strRequest.find("CSeq: ");
    int pos1 = strRequest.find("\r\n", pos);

    std::string strCSeq = strRequest.substr(pos+6, pos1-pos-6);
//    printf("CSeq=%s \n", strCSeq.c_str());

    int result = 0;
    if( method == "OPTIONS" )
    {
        int errCode = 200;
        if( access(rtsp_demux->filename.c_str(), 0) != 0 )
        {
            errCode = 404;
            result = -1;
        }

        rtsp_demux->response = getResponse_OPTIONS(errCode, "Robert RTSPServer", strCSeq);
    }
    else if( method == "DESCRIBE" )
    {
        rtsp_demux->response = getResponse_DESCRIBE(rtsp_demux->filenameext, strCSeq);
    }
    else if( method == "SETUP" )
    {
        pos = strRequest.find("RTP/AVP/TCP");
//        printf("setup pos = %d \n", pos);

        if(  pos > 0 )
        {
            rtsp_demux->transport_protocol = 1;
            rtsp_demux->response = getResponse_SETUP_TCP(rtsp_demux->client_session_id.c_str(), strCSeq);            
        }
        else
        {
            rtsp_demux->transport_protocol = 0;
            pos = strRequest.find("client_port=");
            pos1 = strRequest.find(pos);
            std::string strTemp = strRequest.substr(pos+strlen("client_port="),pos1-pos);
            rtsp_demux->client_rtp_port = atoi(strTemp.c_str());
            rtsp_demux->response = getResponse_SETUP_UDP(rtsp_demux->client_session_id.c_str(), atoi(strTemp.c_str()), rtsp_demux->server_rtp_port, strCSeq);            
        }
    }
    else if( method == "PLAY" )
    {
        rtsp_demux->response = getResponse_PLAY(rtsp_demux->client_session_id.c_str(), strCSeq);        
        canSend = 1;
        transport_proto = rtsp_demux->transport_protocol;
    }
    else if( method == "PAUSE" )
    {
        rtsp_demux->response = getResponse_PAUSE(rtsp_demux->client_session_id.c_str(), strCSeq);        
    }
    else if( method == "TEARDOWN" )
    {
        rtsp_demux->response = getResponse_TEARDOWN(rtsp_demux->client_session_id.c_str(), strCSeq);        
    }
    else
    {
        printf("unknown method\n");
        result = -1;
    }
    *response = rtsp_demux->response.c_str();

    return result;
}

int rtsp_demux_get_client_rtp_port(void* handle)
{
    rtsp_demux_desc_t* rtsp_demux = (rtsp_demux_desc_t*)handle;

    return rtsp_demux->client_rtp_port;
}

int rtsp_demux_close(void* handle)
{
    rtsp_demux_desc_t* rtsp_demux = (rtsp_demux_desc_t*)handle;
    delete rtsp_demux;

    return 0;
}