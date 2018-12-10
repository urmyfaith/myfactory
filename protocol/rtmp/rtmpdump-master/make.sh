gcc rtmpdump.c -DRTMPDUMP_VERSION="1.0" -c -o rtmpdump.o
gcc thread.c -DRTMPDUMP_VERSION="1.0" -c -o thread.o
gcc rtmpdump.o thread.o -o rtmpdump -L./librtmp/ -lrtmp

gcc rtmpsuck.c -DRTMPDUMP_VERSION="1.0" -c -o rtmpsuck.o
gcc rtmpsuck.o thread.o -o rtmpsuck -L./librtmp/ -lrtmp

gcc rtmpgw.c -DRTMPDUMP_VERSION="1.0" -c -o rtmpgw.o
gcc rtmpgw.o thread.o -o rtmpgw -L./librtmp/ -lrtmp

gcc rtmpsrv.c -DRTMPDUMP_VERSION="1.0" -c -o rtmpsrv.o
gcc rtmpsrv.o thread.o -o rtmpsrv -L./librtmp/ -lrtmp
