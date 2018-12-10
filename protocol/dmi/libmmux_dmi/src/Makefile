
CC = g++
CFLAGS = -g -Wall -O3 -fPIC
INCLUDES = -I/usr/local/include
LDFLAGS = -L/usr/local/lib
PROJECT = mmux_dmi
SRC = packer.cpp\
	parser.cpp\
	mmux_dmi.cpp\
	utils.cpp

OBJ = $(SRC:%.cpp=%.o)

all: $(OBJ) static_lib shared_lib mvtodir

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

static_lib: $(OBJ)
	ar -r lib$(PROJECT).a $(OBJ)

shared_lib: $(OBJ)
	$(CC) -fPIC -shared -o lib$(PROJECT).so $(OBJ) $(LDFLAGS)

mvtodir: static_lib
	mkdir -p ../release/include
	mkdir -p ../release/lib/$(shell uname)-$(shell uname -m)
	mv lib$(PROJECT).a ../release/lib/$(shell uname)-$(shell uname -m)/
	mv lib$(PROJECT).so ../release/lib/$(shell uname)-$(shell uname -m)/
	cp mmux_dmi.h ../release/include/
	cp mcodec.h ../release/include/

install:
	mkdir -p /usr/local/include/mmux_dmi
	cp ../release/include/* /usr/local/include/mmux_dmi/
	cp ../release/lib/$(shell uname)-$(shell uname -m)/* /usr/local/lib/

clean:
	clear
	rm -rf $(OBJ) -r libdmi.a libdmi.so ../release
