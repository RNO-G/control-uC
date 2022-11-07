CC = cc
CFLAGS=-Wall -O3 -g -Wextra

default: error
	$(CC) $(CFLAGS) error.o -o client client.c
	$(CC) $(CFLAGS) -pthread error.o -o server server.c

<<<<<<< HEAD:client/Makefile
error:
	$(CC) $(CFLAGS) -c error.c
=======
PROGS=rno-g-controller-loader rno-g-controller-extract
>>>>>>> master:loader/Makefile

clean:
	rm -rf error.o client server

<<<<<<< HEAD:client/Makefile
=======
all: $(PROGS) 

clean: 
	rm -f $(PROGS) 

rno-g-controller-loader: rno-g-controller-loader.c ../shared/base64.c 
	$(CC) -o $@  $(CFLAGS) -D_HOST_ $^ -I../

rno-g-controller-extract: rno-g-controller-extract.c ../shared/base64.c 
	$(CC) -o $@  $(CFLAGS) -D_HOST_ $^ -I../

install: 
	install rno-g-controller-loader $(PREFIX)/bin
	install rno-g-controller-extract $(PREFIX)/bin

uninstall: 
	rm -f $(PREFIX)/bin/rno-g-controller-loader
	rm -f $(PREFIX)/bin/rno-g-controller-extract
	
>>>>>>> master:loader/Makefile
