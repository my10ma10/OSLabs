.PHONY: all clean

all: mychmod.o
	gcc mychmod.o -o mychmod

mychmod.o: mychmod.c
	gcc -c -Wall -Wextra mychmod.c

clean:
	rm -rf *.o mychmod