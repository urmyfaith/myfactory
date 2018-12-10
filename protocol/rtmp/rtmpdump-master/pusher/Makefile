CXX = g++
OBJS = main.o rtmpstream.o h264framer.o
CXXFLAGS = -W -Wall -O2 -g -I../librtmp/
LIBS =  -L../librtmp/ -lrtmp

publisher: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIBS) 

clean:
	rm -rf *.o publisher