CXX = g++
OBJS = main.o amf.o utils.o
CXXFLAGS = -g

server: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)


clean:
	rm -rf *.o server
