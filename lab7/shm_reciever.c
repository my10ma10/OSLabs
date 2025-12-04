#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

#define SHM_SIZE 512
#define BUF_SIZE 1024

struct tm get_time();

const char * shm_name = "SHM_FILE";

int main() {
    
    int fd = open(shm_name, O_CREAT | O_RDONLY, 0600);
    if (fd == -1) {
        perror("open");
    }

    key_t shm_key = ftok(shm_name, 2);
    if (shm_key == -1) {
        fprintf(stderr, "ftok %s\b", strerror(errno));
        close(fd);
        return 1;
    }
    close(fd);

    int shm_id = shmget(shm_key, SHM_SIZE * sizeof(char), IPC_CREAT | 0600);
    if (shm_id == -1) {
        fprintf(stderr, "shmget %s\b", strerror(errno));
        close(shm_id);
        return 1;
    }

    char* sh_array = shmat(shm_id, NULL, 0);
    if ((int)*sh_array == -1) {
        perror("shmat");
        return 1;
    }

    if (strlen(sh_array) == 0) {
        printf("[RECIEVER] В разделяемой памяти пока ничего нет\n");
    }
    else {
        printf("[RECIEVER] Получено сообщение: %s\n", sh_array);
    }

    struct tm time_info = get_time();

    char pid_time[BUF_SIZE];
    sprintf(pid_time, "\n%spid = %d\n", asctime(&time_info), getpid());

    printf("[RECIEVER] Текущее время и pid: %s\n", pid_time);

    if (shmdt(sh_array) == -1 ) {
        perror("shmdt");
        return 1;
    }

    return 0;
}

struct tm get_time() {
    time_t cur_time;
    time(&cur_time);
    struct tm time_info = *localtime(&cur_time);

    return time_info;
}