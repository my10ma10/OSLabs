.PHONY: all clean

all: shm_reciever shm_sender

shm_reciever: shm_reciever.c
	gcc shm_reciever.c -o shm_reciever -lpthread -Wall -Wextra

shm_sender: shm_sender.c
	gcc shm_sender.c -o shm_sender -lpthread -Wall -Wextra


clean: 
	rm -rf *.o shm_reciever shm_sender