CXX = g++
OBJS = ../../protocol/librtp/rtph264.o \
		../../protocol/librtp/rtpaac.o \
		../../protocol/librtp/rtpraw.o \
		../../protocol/librtp/rtp.o \
		../../protocol/librtsp/rtspdemux.o \
		../../file/libh26x/h264demux.o \
		../../file/libpcm/pcmdemux.o \
		../../file/libh26x/spspps.o \
		../../file/libaac/aacdemux.o \
		../../transport/tcp/tcpserver.o \
		../../transport/udp/udpclient.o \
		rtspserver.o \
		demuxer.o \
		muxer.o \
		main.o

CXXFLAGS = -g -W -Wall -std=c++11 -lpthread -Ithirdparty/lib/libst
LIBS = thirdparty/lib/libst/libst.a -lstdc++

service: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIBS) 

clean:
	rm -rf $(OBJS) service
