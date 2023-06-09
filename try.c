#include "codec.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_THREADS 8
#define BLOCK_SIZE 1024

typedef struct {
    char *data;
    int key;
    int index; // added index to keep track of the block order
} Block;

typedef struct {
    Block *blocks;
    int size;
    int capacity;
    int next;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} ThreadPool;

void init_thread_pool(ThreadPool *pool, int capacity) {
    pool->size = 0;
    pool->capacity = capacity;
    pool->next = 0;
    pool->blocks = (Block*) malloc(capacity * sizeof(Block));
    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->not_empty, NULL);
    pthread_cond_init(&pool->not_full, NULL);
}

void destroy_thread_pool(ThreadPool *pool) {
    free(pool->blocks);
    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->not_empty);
    pthread_cond_destroy(&pool->not_full);
}

void add_block(ThreadPool *pool, Block block) {
    pthread_mutex_lock(&pool->lock);
    while (pool->size == pool->capacity) {
        pthread_cond_wait(&pool->not_full, &pool->lock);
    }
    pool->blocks[pool->size++] = block;
    pthread_cond_signal(&pool->not_empty);
    pthread_mutex_unlock(&pool->lock);
}

Block get_block(ThreadPool *pool) {
    pthread_mutex_lock(&pool->lock);
    while (pool->size == 0) {
        pthread_cond_wait(&pool->not_empty, &pool->lock);
    }
    Block block = pool->blocks[--pool->size];
    pthread_cond_signal(&pool->not_full);
    pthread_mutex_unlock(&pool->lock);
    return block;
}

void* worker(void *arg) {
    ThreadPool *pool = (ThreadPool*) arg;
    while (1) {
        Block block = get_block(pool);
        encrypt(block.data, block.key);
        printf("Block %d: %s\n", block.index, block.data); // print block data with index
        fflush(stdout); // flush stdout to ensure output order
    }
    return NULL;
}
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: key < file \n");
        printf("!! data more than 1024 char will be ignored !!\n");
        return 0;
    }

    int key = atoi(argv[1]);
    printf("key is %i \n", key);

    ThreadPool pool;
    init_thread_pool(&pool, MAX_THREADS);

    pthread_t threads[MAX_THREADS];
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_create(&threads[i], NULL, worker, &pool);
        pthread_detach(threads[i]);
    }

    char c;
    int counter = 0;
    char block_data[BLOCK_SIZE];
    char *output_data = NULL; // Initialize the pointer to the output data
    int output_size = 0; // Initialize the size of the output data to 0
    int current_block_index = 0;

    while ((c = getchar()) != EOF) {
        block_data[counter++] = c;
        if (counter == BLOCK_SIZE) {
            Block* block = (Block*)malloc(sizeof(Block));
            block->data = block_data;
            block->key = key;
            block->index = current_block_index++;
            add_block(&pool, *block);
            counter = 0;
            block_data[0] = '\0';
        }
    }

    // Check if there is any data left to encrypt
    if (counter > 0) {
        Block block;
        block.data = block_data;
        block.key = key;
        add_block(&pool, block);
    }

    // Wait for the threads to finish encrypting all the data
    while (pool.size > 0) {
        Block block = get_block(&pool);
        encrypt(block.data, block.key);

        // Add the encrypted data to the output buffer
        output_data = realloc(output_data, output_size + strlen(block.data) + 1);
        strcpy(output_data + output_size, block.data);
        output_size += strlen(block.data);
    }

    // Print the encrypted output in order
    printf("encrypted data:\n %s\n", output_data);

    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_cancel(threads[i]);
    }

    destroy_thread_pool(&pool);

    return 0;
}
