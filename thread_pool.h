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
    // printf("1\n");
    // fflush(stdout);
    // if (my_task == NULL)
    // {
    //     printf("Error: invalid task");
    //     fflush(stdout);
    // }

    if (my_task->job == ENCRYPT)
    {
        printf("encripted data:\n %s\n", my_task->data);
    }
    else
    {
        printf("decrypted data:\n %s\n", my_task->data);
    }
}

void handle_task(ThreadPool *pool)
{
    // printf("2\n");
    // fflush(stdout);
    // if (pool == NULL)
    // {
    //     printf("Error: invalid pool");
    //     fflush(stdout);
    // }

    Task *my_task = get_need_thread_to_exacute(pool->queue);
    
    // if (my_task == NULL)
    // {
    //     printf("Error: invalid current task");
    //     fflush(stdout);
    // }

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
    print_task(my_task);
    dequeue(pool->queue);
}

void *thread_handler(void *arg)
{
    // printf("3\n");
    // fflush(stdout);
    ThreadHandlerArg *argPool = (ThreadHandlerArg *)arg;
    ThreadPool *pool = argPool->pool;
    int index = argPool->index;
    // printf("\n\npid = %ld\n\nthread = %ld\n\n",pthread_self(),pool->threads[index].thread);
    // int bool = 1;
    while (1)
    {
        // if (&pool->threads[index].cond == NULL)
        // {
        //     printf("Error: invalid pool cond");
        //     fflush(stdout);
        // }

        // if (&pool->threads[index].bussy_lock == NULL)
        // {
        //     printf("Error: invalid pool lock");
        //     fflush(stdout);
        // }

        pthread_cond_wait(&pool->threads[index].cond, &pool->threads[index].bussy_lock);
        if (pool->queue->size > 0)
        {
            handle_task(pool);
            printf("Thread %ld finished.\n", (long)pthread_self());
            pool->threads[index].is_bussy = 0;
        }

        // close after finish
        if (pool->queue->size == 0)
        {
            printf("\nCLOSE\n");
            for (int i = 0; i < pool->pool_size; i++)
            {
                pthread_exit(&pool->threads[i].thread);
            }
            // bool = 0;
        }
        // pthread_mutex_unlock(&pool->queue->lock);
    }
    printf("\nENDDD\n");
}

void init_thread_pool(ThreadPool *pool)
{
    pool->queue = (Queue *)malloc(sizeof(Queue));
    init_queue(pool->queue);
    pool->pool_size = ((int)sysconf(_SC_NPROCESSORS_ONLN) * 2) - 1; // number of cores
    printf("I have %d cores\n", pool->pool_size);
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

        // pthread_detach(pool->threads[i]);
    }
}
