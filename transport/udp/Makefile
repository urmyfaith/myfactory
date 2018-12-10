CXX = g++
CXXFLAGS = -g
LIBS = -lpthread

TARGET0 = udpclienttest
TARGET1 = udpservertest
TARGETS = $(TARGET0) $(TARGET1) 

OBJS0 = udpclient.o
OBJS1 = udpserver.o

$(TARGET0):$(OBJS0) $(TARGET0).o
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS0) $(TARGET0).o $(LIBS) 

$(TARGET1):$(OBJS1) $(TARGET1).o
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS1) $(TARGET1).o $(LIBS) 

clean:
	clear
	rm -rf $(OBJS0) $(OBJS1) *.o $(TARGETS)
