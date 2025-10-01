.PHONY: all clean

all: myls

myls: myls.o
	gcc myls.o -o myls

myls.o: myls.c
	gcc myls.c -c -Wall -Wextra

clean: 
	rm -rf *.o myls