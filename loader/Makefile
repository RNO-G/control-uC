CFLAGS?=-Wall -O3 -g -Wextra

PROGS=rno-g-controller-loader rno-g-controller-extract

RNO_G_INSTALL_DIR?=/rno-g
PREFIX?=$(RNO_G_INSTALL_DIR)
DESTDIR?=/

all: $(PROGS)

clean:
	rm -f $(PROGS)

rno-g-controller-loader: rno-g-controller-loader.c ../shared/base64.c
	$(CC) -o $@  $(CFLAGS) -D_HOST_ $^ -I../

rno-g-controller-extract: rno-g-controller-extract.c ../shared/base64.c
	$(CC) -o $@  $(CFLAGS) -D_HOST_ $^ -I../

install:
	install rno-g-controller-loader ${DESTDIR}/$(PREFIX)/bin
	install rno-g-controller-extract ${DESTDIR}/$(PREFIX)/bin

uninstall:
	rm -f ${DESTDIR}/$(PREFIX)/bin/rno-g-controller-loader
	rm -f ${DESTDIR}/$(PREFIX)/bin/rno-g-controller-extract
