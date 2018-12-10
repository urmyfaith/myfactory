gcc amf.c -fPIC -c -o amf.o
gcc log.c -c -fPIC -o log.o
gcc parseurl.c -c -fPIC -o parseurl.o
gcc rtmp.c -DNO_CRYPTO -c -fPIC -o rtmp.o
gcc hashswf.c -DNO_CRYPTO -c -fPIC -o hashswf.o

ar -r librtmp.a *.o
gcc *.o -shared -fPIC -o librtmp.so

#gcc ../rtmpdump.c -o rtmpdump
