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
    pthread_mutex_t pool_lock;
    int key;
    int job;
} ThreadPool;

typedef struct ThreadHandlerArg
{
    int index;
    ThreadPool *pool;
} ThreadHandlerArg;

void free_thread_pool(ThreadPool *pool);
void print_task(Task *my_task);
void handle_task(ThreadPool *pool);
void *thread_handler(void *arg);
void init_thread_pool(ThreadPool *pool);