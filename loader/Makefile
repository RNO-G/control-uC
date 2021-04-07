CFLAGS=-Wall -O3 -g


PROGS=rno-g-controller-loader rno-g-controller-extract

all: $(PROGS) 

clean: 
	rm -f $(PROGS) 

rno-g-controller-loader: rno-g-controller-loader.c ../shared/base64.c 
	$(CC) -o $@  $(CFLAGS) -D_HOST_ $^ -I../

rno-g-controller-extract: rno-g-controller-extract.c ../shared/base64.c 
	$(CC) -o $@  $(CFLAGS) -D_HOST_ $^ -I../
