#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <sys/types.h>

#define ARR_SIZE 10
#define THR_COUNT 10

pthread_mutex_t mtx;

void* read_func(void* arg) {
    int* mas = (int*)arg;
    
    while (mas[9] == 0) {
        pthread_mutex_lock(&mtx);

        printf("READ_THR #%ld] mas: ", pthread_self());
        
        for (int i = 0; i < ARR_SIZE; ++i) {
            printf("%d ", mas[i]);
        }
        printf("\n");

        pthread_mutex_unlock(&mtx);
        usleep(100);
    }

    pthread_exit(NULL);
}

void* write_func(void* arg) {
    int* mas = (int*)arg;
    

    for (int i = 0; i < ARR_SIZE; ++i) {
        pthread_mutex_lock(&mtx);
        mas[i] = i + 1;
        pthread_mutex_unlock(&mtx);
        usleep(1);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t write_tid;
    pthread_t read_tids[10];

    int res = pthread_mutex_init(&mtx, NULL);
    if (res == -1) {
        perror("pthread_mutex_init");
        exit(1);
    }

    int* mas = calloc(sizeof(int), ARR_SIZE);

    pthread_create(&write_tid, NULL, write_func, mas);

    for (int i = 0; i < THR_COUNT; ++i) {
        pthread_create(&read_tids[i], NULL, read_func, mas);
    }

    pthread_join(write_tid, NULL);
    for (int i = 0; i < THR_COUNT; ++i) {
        pthread_join(read_tids[i], NULL);
    }

    pthread_mutex_destroy(&mtx);
    
    free(mas);
    return 0;
} 