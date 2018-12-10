CXX = g++
CXXFLAGS = -g
LIBS =  -L/usr/local/lib -lmp4v2

TARGET0 = h264tomp4
TARGET1 = pcmtomp4
TARGET2 = pcmtowav
TARGET3 = h264tops
TARGET4 = h264tots
TARGET5 = h264toflv
TARGET6 = aactoflv
TARGET7 = aactomp4
TARGET8 = h264tots1
TARGETS = $(TARGET0) $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5) \
		$(TARGET6) $(TARGET7) $(TARGET8)

OBJS = libh26x/h264demux.o libmp4/mp4mux.o libh26x/spspps.o \
		libaac/aacdemux.o libflv/flvmux.o libmpeg/pes.o \
		libmpeg/psdemux.o libmpeg/psmux.o libmpeg/tsdemux.o libmpeg/tsmux.o \
		libpcm/pcmdemux.o	libwav/wavfile.o libmpeg/MuxTs.o


$(TARGET0):$(OBJS) $(TARGET0).o
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(TARGET0).o $(LIBS) 

$(TARGET1):$(OBJS) $(TARGET1).o
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(TARGET1).o $(LIBS) 

$(TARGET2):$(OBJS) $(TARGET2).o
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(TARGET2).o $(LIBS) 

$(TARGET3):$(OBJS) $(TARGET3).o
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(TARGET3).o $(LIBS) 

$(TARGET4):$(OBJS) $(TARGET4).o
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(TARGET4).o $(LIBS) 

$(TARGET5):$(OBJS) $(TARGET5).o
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(TARGET5).o $(LIBS) 

$(TARGET6):$(OBJS) $(TARGET6).o
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(TARGET6).o $(LIBS) 

$(TARGET7):$(OBJS) $(TARGET7).o
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(TARGET7).o $(LIBS) 

$(TARGET8):$(OBJS) $(TARGET8).o
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(TARGET8).o $(LIBS) 

clean:
	clear
	rm -rf $(OBJS) *.o $(TARGETS)
