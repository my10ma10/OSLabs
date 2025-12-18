#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#define SHM_SIZE 1024
#define BUF_SIZE 1024

void sem_lock(int sem_id);
void sem_unlock(int sem_id);
struct tm get_time();

void exit_handler();

const char* shm_name = "SHM_NAME";

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
    if ((char)*sh_array == -1) {
        perror("shmat");
        unlink(shm_name);
        return 1;
    }


    int sem_id = semget(shm_key, 1, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("semget");
        return 1;
    }

    semctl(sem_id, 0, SETVAL, 1);



    while (1) {
        struct tm time_info = get_time();
        
        char cur_time[BUF_SIZE];
        sprintf(cur_time, "%spid = %d\n", asctime(&time_info), getpid());
        
        char msg[BUF_SIZE];
        snprintf(msg, SHM_SIZE * 2, "SENDER #%d: %s", getpid(), cur_time);
        /*lock*/
        sem_lock(sem_id);

        strcpy(sh_array, msg);

        sem_unlock(sem_id);
        /*unlock*/


        sleep(3);
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

void sem_lock(int sem_id) {
    struct sembuf op = {0, -1, 0};
    semop(sem_id, &op, 1);
}

void sem_unlock(int sem_id) {
    struct sembuf op = {0, 1, 0};
    semop(sem_id, &op, 1);
}


void exit_handler() {
    unlink(shm_name);
    exit(1);
}