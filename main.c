#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "threadPool.h"

void task_func(void* arg) {
    int num = *(int*)arg;
    printf("Task %d started\n", num);
    sleep(1);
    printf("Task %d finished\n", num);
}

int main() {
    thread_pool_t pool;
    thread_pool_init(&pool);

    for (int i = 0; i < 8; ++i) {
        int* arg = (int*) malloc(sizeof(int));
        *arg = i;
        enqueue(&pool, task_func, arg);
    }

    // thread_pool_destroy(&pool);
    return 0;
}
