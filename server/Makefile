all: server

server: main.o server.o ../connect6_protocol/libconnect6_protocol.so
	gcc -o server main.o server.o -L../connect6_protocol -lconnect6_protocol

clean:
	rm -f server *.o
