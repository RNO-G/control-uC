CC = cc
CFLAGS = -Wall -O3 -g -Wextra

RNO_G_INSTALL_DIR?=/home/sharada/rno-g/bin
PREFIX?=$(RNO_G_INSTALL_DIR)

default:
	$(CC) $(CFLAGS) -pthread -o console console.c main.c

clean:
	rm -rf console

install:
	install console $(PREFIX)

uninstall:
	rm -rf $(PREFIX)/console

