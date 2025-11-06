.PHONY: all clean

all: archiver

debug: 
	gcc archive.c -g -Wall -Wextra -o debug_arch

archiver: archive.o
	gcc archive.o -o archiver

archive.o: archive.c
	gcc archive.c -c -Wall -Wextra

clean: 
	rm -rf *.o archiver debug_arch