CC = cc
CFLAGS = -Wall -O3 -g -Wextra

RNO_G_INSTALL_DIR?=/rno-g
PREFIX?=$(RNO_G_INSTALL_DIR)

default:
	$(CC) $(CFLAGS) -pthread -o server client.c server.c main.c

clean:
	rm -rf server

install:
	install server $(PREFIX)/bin

uninstall:
	rm -rf $(PREFIX)/bin/server

