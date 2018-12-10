CXX = g++
CXXFLAGS = -g -I../../libaac -I../../libh26x
LIBS =  -L/usr/local/lib 

TARGET0 = tstest

TARGETS = $(TARGET0)

OBJS = ../../libh26x/h264demux.o ../../libh26x/spspps.o \
		../../libaac/aacdemux.o tsmux2.o \
		utils.o \
		main.o

$(TARGET0):$(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIBS) 

clean:
	clear
	rm -rf $(OBJS) *.o $(TARGETS) *.ts
