CXX = g++
CXXFLAGS = -g
LIBS = -lpthread

TARGET0 = multidest
TARGETS = $(TARGET0)

OBJS0 = multidest_mux.o

$(TARGET0):$(OBJS0)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS0) $(LIBS) 

clean:
	clear
	rm -rf $(OBJS0) *.o $(TARGETS)
