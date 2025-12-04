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
#include <signal.h>

#define SHM_SIZE 512
#define BUF_SIZE 1024

struct tm get_time();
void exit_handler();
void exit_handler();

const char * shm_name = "SHM_FILE";

int main() {
    signal(SIGINT, exit_handler);
    
    int fd = open(shm_name, O_CREAT | O_EXCL | O_WRONLY, 0600);
    if (fd == -1) {
        if (errno == EEXIST) {
            fprintf(stderr, "Программа уже запущена или файл уже существует\n");
        }
        else {
            perror("open");
            unlink(shm_name);
        }
        return 1;
    }

    key_t shm_key = ftok(shm_name, 2);
    if (shm_key == -1) {
        fprintf(stderr, "ftok %s\b", strerror(errno));
        close(fd);
        unlink(shm_name);
        return 1;
    }
    close(fd);

    int shm_id = shmget(shm_key, SHM_SIZE * sizeof(char), IPC_CREAT | 0600);
    if (shm_id == -1) {
        fprintf(stderr, "shmget %s\b", strerror(errno));
        close(shm_id);
        unlink(shm_name);
        return 1;
    }

    char* sh_array = shmat(shm_id, NULL, 0);
    if ((int)*sh_array == -1) {
        perror("shmat");
        unlink(shm_name);
        return 1;
    }


    while (1) {
        struct tm time_info = get_time();

        char pid_time[BUF_SIZE];
        sprintf(pid_time, "\n%spid = %d\n", asctime(&time_info), getpid());
        strcpy(sh_array, pid_time);

        printf("[SENDER] Отправлено сообщение: %s", pid_time);

        sleep(1);
    }

    if (shmdt(sh_array) == -1 ) {
        perror("shmdt");
        return 1;
    }

    shmctl(shm_id, IPC_RMID, NULL);
    unlink(shm_name);

    return 0;
}

struct tm get_time() {
    time_t cur_time;
    time(&cur_time);
    struct tm time_info = *localtime(&cur_time);

    return time_info;
}


void exit_handler() {
    unlink(shm_name);
    exit(1);
}