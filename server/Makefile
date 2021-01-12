all: server

server: main.o ../connect6_protocol/libconnect6_protocol.a
	gcc -o server main.o -L../connect6_protocol -lconnect6_protocol

clean:
	rm -f server *.o
