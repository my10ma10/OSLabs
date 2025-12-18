#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#define ARR_SIZE 10

void* write_func(void* arg);
void* read_func(void* arg);

sem_t g_sem;

static int g_counter = 0;

int main() {
    pthread_t write_tid;
    pthread_t read_tid;

    int* mas = calloc(sizeof(int), ARR_SIZE);
    if (!mas) {
        perror("calloc");
        exit(1);
    }

    if (sem_init(&g_sem, 0, 1) == -1) {
        perror("sem_init");
        exit(1);
    }

    pthread_create(&write_tid, NULL, write_func, mas);
    pthread_create(&read_tid, NULL, read_func, mas);

    sleep(5);


    pthread_join(write_tid, NULL);
    pthread_join(read_tid, NULL);

    sem_destroy(&g_sem);
    free(mas);
    return 0;
}



void* write_func(void* arg) {
    int* mas = (int*)arg;
    
    while (1) {

        if (g_counter == 10) {
            sem_post(&g_sem);
            break;
        }

        mas[g_counter] = g_counter + 1;
        g_counter++;


        printf("[%d. WRITER #%ld]: { ", g_counter, pthread_self());
        for (int i = 0; i < g_counter; ++i) {
            printf("%d ", mas[i]);
        }
        printf("}\n");

        sem_post(&g_sem);

        sleep(1);
    }

    pthread_exit(NULL);
}


void* read_func(void* arg) {
    int* mas = (int*)arg;

    
    while (1) {
        sem_wait(&g_sem);

        if (g_counter == 10) {
            sem_post(&g_sem);
            break;
        }

        printf("[%d. READER #%ld]: { ", g_counter, pthread_self());
        for (int i = 0; i < g_counter; ++i) {
            printf("%d ", mas[i]);
        }
        printf("}\n");
    }

    pthread_exit(NULL);
}