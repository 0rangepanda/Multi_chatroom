all: server client

server: server.o
	${CC} -o server -g server.o -lpthread

server.o: server.c
	${CC} -g -c -Wall server.c -lpthread

client: client.o
	${CC} -o client -g client.o

client.o: client.c
	${CC} -g -c -Wall client.c


clean:
	rm -f *.o *~ server client
