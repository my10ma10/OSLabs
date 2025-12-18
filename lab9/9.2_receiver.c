#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#define SHM_SIZE 1024
#define BUF_SIZE 1024

void sem_lock(int sem_id);
void sem_unlock(int sem_id);
struct tm get_time();


const char* shm_name = "SHM_NAME";

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
    if ((char)*sh_array == -1) {
        perror("shmat");
        return 1;
    }

    int sem_id = semget(shm_key, 1, 0666);
    if (sem_id == -1) {
        perror("semget");
        return 1;
    }

    while (1) {
        struct tm time_info = get_time();
        
        char cur_time[BUF_SIZE];
        sprintf(cur_time, "%spid = %d\n", asctime(&time_info), getpid());

        char msg[BUF_SIZE];

        sem_lock(sem_id);
        
        printf("RECEIVER #%d: %s", getpid(), cur_time);
        strcpy(msg, sh_array);

        printf("RECEIVER: new message: '%s'", msg);
    }

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

void sem_lock(int sem_id) {
    struct sembuf op = {0, -1, 0};
    semop(sem_id, &op, 1);
}

void sem_unlock(int sem_id) {
    struct sembuf op = {0, 1, 0};
    semop(sem_id, &op, 1);
}