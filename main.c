#include "thread_pool.h"

#define BLOCK_SIZE 1024

void signal_thread(ThreadPool *pool, int thread_index)
{
    pthread_cond_signal(&(pool->threads[thread_index].cond));
}

int wake_free_thread(ThreadPool *pool)
{
    int assigned = 0;
    while (assigned == 0)
    {
        for (int i = 0; i < pool->pool_size; i++)
        {
            pthread_mutex_lock(&(pool->threads[i].bussy_lock));
            if (pool->threads[i].is_bussy == 0)
            {
                pool->threads[i].is_bussy = 1;
                assigned = 1;
                signal_thread(pool, i);
                pthread_mutex_unlock(&(pool->threads[i].bussy_lock));
                return i;
            }
            pthread_mutex_unlock(&(pool->threads[i].bussy_lock));
        }
    }
}

void wake_all_thread(ThreadPool *pool)
{
    while (pool->queue->size > 0)
    {
        for (int i = 0; i < pool->pool_size; i++)
        {
            pthread_mutex_lock(&(pool->threads[i].bussy_lock));
            if (pool->threads[i].is_bussy == 0)
            {
                pool->threads[i].is_bussy = 1;
                signal_thread(pool, i);
            }
            pthread_mutex_unlock(&(pool->threads[i].bussy_lock));
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("usage: key < file \n");
        printf("!! data more than 1024 char will be ignored !!\n");
        return 0;
    }

    int key = atoi(argv[1]);
    printf("key is %i \n", key);

    int oper = -1;
    if (!strcmp(argv[2], "-e") || !strcmp(argv[2], "-d"))
    {
        if (!strcmp(argv[2], "-e"))
        {
            oper = ENCRYPT;
        }
        else
        {
            oper = DECRYPT;
        }
    }
    else
    {
        printf("Error: flag not valid. use -e or -d");
        return 0;
    }

    ThreadPool *pool;
    pool = (ThreadPool *)malloc(sizeof(ThreadPool));
    init_thread_pool(pool);
    pool->key = key;
    pool->job = oper;
    char c;
    int counter = 0;
    char block_data[BLOCK_SIZE];
    memset(block_data, 0, BLOCK_SIZE);
    char *output_data = NULL; // Initialize the pointer to the output data
    int output_size = 0;      // Initialize the size of the output data to 0

    while ((c = getchar()) != EOF)
    {
        block_data[counter++] = c;
        if (counter == BLOCK_SIZE)
        {
            enqueue(pool->queue, block_data); // need to know if its encrypt or decrypt <<<<<<<<--------------------------------------
            counter = 0;
            // block_data[0] = '\0';
            memset(block_data, 0, BLOCK_SIZE);
        }
    }
    // Check if there is any data left to encrypt
    if (counter > 0)
    {
        enqueue(pool->queue, block_data);
    }
    wake_all_thread(pool);

    // Wait for the threads to finish processing all the data
    for (int i = 0; i < pool->pool_size; i++)
    {
        pthread_join(pool->threads[i].thread, NULL);
    }
    free_thread_pool(pool);
    return 0;
}
