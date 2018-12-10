CC=gcc
CFLAGS=-c -Wall

all: example

example: example.o g711.o g711_table.o
	$(CC) example.o g711.o g711_table.o -o example

example.o: example.c
	$(CC) $(CFLAGS) example.c

g711_table.o: g711_table.c
	$(CC) $(CFLAGS) g711_table.c

g711.o: g711.c
	$(CC) $(CFLAGS) g711.c

clean:
	rm *.o example
