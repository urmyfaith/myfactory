C 			 = c
CPP 		 = cpp
CXX = gcc
CXXFLAGS = -w -g -D__STDC_CONSTANT_MACROS -I ../tsmux/
LIBS =  -lstdc++ -lpthread -lavformat -lavcodec -lavfilter -lswscale -lswresample -lavutil -lavfilter -lm 

TARGET0 = testaac

TARGETS = $(TARGET0) 

CPPOBJS = aacdemux.o \
		main.o \
		tsfile2.o  \
		pes.o \
		tsmux.o \
		h264demux.o \
		spspps.o \
		ffmpeg_mux.o \
		generator_test.o \
		audio_generator.o \
		video_generator.o \
		ts_test.o \
		tsfile.o \
		ffmpeg_enc.o \
		ffmpegresample.o \
		ffmpeg_filter.o


$(TARGET0):$(CPPOBJS) $(COBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(CPPOBJS)  $(COBJS) $(LIBS) 

clean:
	clear
	rm -rf $(OBJS) *.o $(TARGETS) *.ts
