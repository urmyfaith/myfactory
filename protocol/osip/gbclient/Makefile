CXX = g++
OBJS = main.o
CXXFLAGS = -W -Wall -O2 -g
LIBS = -losip2 -leXosip2

gbclient: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIBS) 

clean:
	clear
	rm -rf *.o gbclient
