CXX = g++
CXXFLAGS = -g -Iinclude/
LIBS =  -Iinclude/

TARGET0 = mp4_demux_test

TARGETS = $(TARGET0) 

OBJS = libmp4/mp4_demux.o libmp4/demux.o

$(TARGET0):$(OBJS) $(TARGET0).o
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(TARGET0).o $(LIBS) 

clean:
	clear
	rm -rf $(OBJS) *.o $(TARGETS)
