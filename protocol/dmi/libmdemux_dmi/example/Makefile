
CC = g++
CFLAGS = -g -Wall -O3 -fPIC
INCLUDES = -I/usr/local/include -I../release/include 
LDFLAGS = -lpthread -L/usr/local/lib -lmdemux_dmi 
PROJECT = dmiclient
SRC = main.cpp


OBJ = $(SRC:%.cpp=%.o)

all: $(OBJ) dmiclient

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

dmiclient: $(OBJ)
	$(CC) $(OBJ) -o $(PROJECT) $(LDFLAGS)

install:

clean:
	clear
	rm -rf $(OBJ) -r *.a *.so dmiclient
