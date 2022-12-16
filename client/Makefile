CC = cc
CFLAGS = -Wall -O3 -g -Wextra

RNO_G_INSTALL_DIR=/home/rno-g
PREFIX=$(RNO_G_INSTALL_DIR)

default: error
	$(CC) $(CFLAGS) error.o -o client client.c
	$(CC) $(CFLAGS) -pthread error.o -o server server.c

error:
	$(CC) $(CFLAGS) -c error.c

clean:
	rm -rf error.o client server

install:
	install client $(PREFIX)/bin
	install server $(PREFIX)/bin

uninstall:
	rm -rf $(PREFIX)/bin/client
	rm -rf $(PREFIX)/bin/server

