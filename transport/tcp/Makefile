CXX = g++
CXXFLAGS = -g
LIBS = -lpthread

TARGET0 = tcpclienttest
TARGET1 = tcpservertest
TARGETS = $(TARGET0) $(TARGET1) 

OBJS0 = tcpclient.o
OBJS1 = tcpserver.o

$(TARGET0):$(OBJS0) $(TARGET0).o
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS0) $(TARGET0).o $(LIBS) 

$(TARGET1):$(OBJS1) $(TARGET1).o
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS1) $(TARGET1).o $(LIBS) 

clean:
	clear
	rm -rf $(OBJS0) $(OBJS1) *.o $(TARGETS)
