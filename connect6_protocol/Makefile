libs: libconnect6_protocol.a

connect6_protocol.o: connect6_protocol.cpp

libconnect6_protocol.a: connect6_protocol.o
	ar rcs libconnect6_protocol.a connect6_protocol.o

clean:
	rm -f *.o *.a *.gch
