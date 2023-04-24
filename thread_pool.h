#include "codec.h"
#include "queue.h"
#include <unistd.h>

typedef struct ThreadPool{
    Queue *queue;
    pthread_t* threads;
    int pool_size;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} ThreadPool;

void print_task(Task* my_task){

    if (my_task->job == ENCRYPT)
    {
        printf("encripted data:\n %s\n",my_task->data);
    }
    else {
        printf("decrypted data:\n %s\n",my_task->data);
    }
}


void handle_task(ThreadPool* pool){
    Task* my_task = get_need_thread_to_exacute(pool->queue);
    if (my_task->job == ENCRYPT)
    {
        encrypt(my_task->data, my_task->key);
    }
    else {
        decrypt(my_task->data, my_task->key);
    }
    // Busy wait need to improve
    while (pool->queue->head != my_task)
    {
        continue;
    }
    print_task(my_task);
    dequeue(pool->queue);
}



void* thread_handler(void* arg) {
    ThreadPool* pool = (ThreadPool*) arg;
    while (1)
    {
        while (pool->queue->size == 0)
        {
            pthread_cond_wait(&pool->cond, &pool->lock);
        }
        handle_task(pool);
        printf("Thread %ld finished.\n",(long)pthread_self());
    }
}

void init_thread_pool(ThreadPool *pool) {
    pool->queue = (Queue*)malloc(sizeof(Queue));
    init_queue(pool->queue);
    pool->pool_size = (int)sysconf(_SC_NPROCESSORS_CONF); // number of cores
    printf("I have %d cores\n", pool->pool_size);
    pool->threads = (pthread_t*)malloc(pool->pool_size * (sizeof(pthread_t)));
    for (int i = 0; i < pool->pool_size; i++)
    {
        pthread_create(&pool->threads[i], NULL, thread_handler, pool);
        pthread_detach(pool->threads[i]);
    }
    
    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->cond, NULL);
}
