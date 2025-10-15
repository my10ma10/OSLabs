.PHONY: all clean

all: mychmod

mychmod: mychmod.o
	gcc mychmod.o -o mychmod

mychmod.o: mychmod.c
	gcc -c mychmod.c -Wall -Wextra

clean:
	rm -rf *.o mychmod