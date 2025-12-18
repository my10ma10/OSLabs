.PHONY: all clean

all: 9.1 9.2_receiver 9.2_sender

9.1: 9.1.c
	gcc 9.1.c -lpthread -Wall -Wextra -o 9.1

9.2_receiver:
	gcc 9.2_receiver.c -lpthread -Wall -Wextra -o 9.2_receiver

9.2_sender:
	gcc 9.2_sender.c -lpthread -Wall -Wextra -o 9.2_sender

clean: 
	rm -f 9.1 9.2_receiver 9.2_sender