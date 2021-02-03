CC=gcc
LDFLAGS=-Wl,-rpath,../connect6_protocol

all: server

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

server: main.o server.o ../connect6_protocol/libconnect6_protocol.so
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -f server *.o
