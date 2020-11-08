SHELL = /bin/sh
CC = gcc

all: efm

efm: efm.o
	$(CC) -o efm efm.o

efm.o: efm.c
	$(CC) -c efm.c


install:
	chmod a+x efm
	cp efm /usr/local/bin/efm

clean:
	rm -rfv *.o
	rm -rfv efm

uninstall:
	rm /usr/local/bin/efm