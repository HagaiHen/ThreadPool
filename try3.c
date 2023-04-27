#include "thread_pool.h"


#define BLOCK_SIZE 1024


void signal_thread(ThreadPool* pool, int thread_index) {
    pthread_mutex_lock(&(pool->lock));
    pthread_cond_signal(&(pool->threads[thread_index].cond));
    pthread_mutex_unlock(&(pool->lock));
}

int wake_free_thread(ThreadPool* pool){
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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: key < file \n");
        printf("!! data more than 1024 char will be ignored !!\n");
        return 0;
    }

    int key = atoi(argv[1]);
    printf("key is %i \n", key);

    ThreadPool* pool;
    pool = (ThreadPool*)malloc(sizeof(ThreadPool));
    init_thread_pool(pool);

    char c;
    int counter = 0;
    char block_data[BLOCK_SIZE];
    char *output_data = NULL; // Initialize the pointer to the output data
    int output_size = 0; // Initialize the size of the output data to 0

    while ((c = getchar()) != EOF) {
        block_data[counter++] = c;
        if (counter == BLOCK_SIZE) {
            enqueue(pool->queue, block_data, key, ENCRYPT);  // need to know if its encrypt or decrypt <<<<<<<<--------------------------------------
            wake_free_thread(pool);
            counter = 0;
            block_data[0] = '\0';
            printf("\ngot a new job!\n");
            fflush(stdout);
        }
    }
    printf("\nfinished1111111111111111111!!!!!!!\n");
    fflush(stdout);
    // Check if there is any data left to encrypt
    if (counter > 0) {
        enqueue(pool->queue, block_data, key, ENCRYPT);  // need to know if its encrypt or decrypt <<<<<<<<--------------------------------------
        pthread_cond_signal(&pool->cond);
    }
    printf("\nfinished22222222222222222222!!!!!!!\n");
    fflush(stdout);
    // printf("\nout1\n");
    // Wait for the threads to finish processing all the data
    for (int i = 0; i < pool->pool_size; i++) {
        pthread_join(pool->threads[i].thread, NULL);
    }
    printf("\nfinished\n");
    fflush(stdout);

//  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< need to wait for everyone(threads) to finish

    // // Wait for the threads to finish encrypting all the data
    // while (pool.size > 0) {
    //     Block block = get_block(&pool);
    //     encrypt(block.data, block.key);

    //     // Add the encrypted data to the output buffer
    //     output_data = realloc(output_data, output_size + strlen(block.data) + 1);
    //     strcpy(output_data + output_size, block.data);
    //     output_size += strlen(block.data);
    // }

    // for (int i = 0; i < MAX_THREADS; i++) {
    //     pthread_cancel(threads[i]);
    // }
// 
    // destroy_thread_pool(&pool); // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    return 0;
}
