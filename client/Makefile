CC = cc
CFLAGS=-Wall -O3 -g -Wextra

default: error
	$(CC) $(CFLAGS) error.o -o client client.c
	$(CC) $(CFLAGS) -pthread error.o -o server server.c

error:
	$(CC) $(CFLAGS) -c error.c

clean:
	rm -rf error.o client server

