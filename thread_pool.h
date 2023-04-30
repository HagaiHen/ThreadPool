#include "codec.h"
#include "queue.h"
#include <unistd.h>

#define ENCRYPT 1
#define DECRYPT 2

typedef struct my_thread
{
    int index;
    pthread_t thread;
    pthread_cond_t cond; // array of condition variables, one for each thread
    pthread_mutex_t bussy_lock;
    int is_bussy; // free = 0, bussy = 1
} my_thread;

typedef struct ThreadPool
{
    Queue *queue;
    my_thread *threads;
    int pool_size;
} ThreadPool;

typedef struct ThreadHandlerArg
{
    int index;
    ThreadPool *pool;
} ThreadHandlerArg;

void print_task(Task *my_task)
{
    if (my_task->job == ENCRYPT)
    {
        printf("%s", my_task->data);
    }
    else
    {
        printf("%s", my_task->data);
    }
}

void handle_task(ThreadPool *pool)
{
    Task *my_task = get_need_thread_to_exacute(pool->queue);

    if (my_task->job == ENCRYPT)
    {
        encrypt(my_task->data, my_task->key);
    }
    else
    {
        decrypt(my_task->data, my_task->key);
    }
    // Busy wait need to improve
    while (pool->queue->head != my_task)
    {
        continue;
    }
    if (pool->queue->size > 0) {
        print_task(my_task);
        dequeue(pool->queue);
        
    } else {
        exit(0);
    }
}

void *thread_handler(void *arg)
{
    ThreadHandlerArg *argPool = (ThreadHandlerArg *)arg;
    ThreadPool *pool = argPool->pool;
    int index = argPool->index;
    while (1)
    {
        pthread_cond_wait(&pool->threads[index].cond, &pool->threads[index].bussy_lock);
        if (pool->queue->size > 0)
        {
            handle_task(pool);
            pool->threads[index].is_bussy = 0;
        }

        // close after finish
        if (pool->queue->size == 0)
        {
            for (int i = 0; i < pool->pool_size; i++)
            {
                pthread_exit(&pool->threads[i].thread);
            }
            exit(0);
        }
    }
}

void init_thread_pool(ThreadPool *pool)
{
    pool->queue = (Queue *)malloc(sizeof(Queue));
    init_queue(pool->queue);
    pool->pool_size = ((int)sysconf(_SC_NPROCESSORS_ONLN) * 2) - 1; // number of cores
    pool->threads = (my_thread *)malloc(pool->pool_size * (sizeof(my_thread)));
    for (int i = 0; i < pool->pool_size; i++)
    {
        pool->threads[i].is_bussy = 0;
        pthread_cond_init(&(pool->threads[i].cond), NULL);
        pthread_mutex_init(&pool->threads[i].bussy_lock, NULL);
        pool->threads[i].index = i;
        ThreadHandlerArg *arg = (ThreadHandlerArg *)malloc(sizeof(ThreadHandlerArg));
        arg->index = i;
        arg->pool = pool;
        pthread_create(&pool->threads[i].thread, NULL, thread_handler, arg);

    }
}
