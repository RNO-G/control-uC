CC = cc
CFLAGS = -Wall -O3 -g -Wextra

RNO_G_INSTALL_DIR?=/rno-g
PREFIX?=$(RNO_G_INSTALL_DIR)

default:
	$(CC) $(CFLAGS) -pthread -o console console.c main.c

clean:
	rm -rf console

install:
	install console $(PREFIX)/bin

uninstall:
	rm -rf $(PREFIX)/bin/console

