# mimimal Makefile

all: server client

CFLAGS = -g

server : server.c common.o -lpthread
client : client.c common.o -lpthread
common.o : common.h

clean: 
	rm server client
