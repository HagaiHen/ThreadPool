#ifndef QUEUE_H
#define QUEUE_H
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define BLOCK_SIZE 1024
#define ENCRYPT 1
#define DECRYPT 2


typedef struct task
{
    char data[BLOCK_SIZE];
    int thread_index;
    struct task *next;
} Task;

typedef struct Queue
{
    Task *head;
    Task *tail;
    int size;
    Task *need_thread_to_exacute;
    pthread_mutex_t lock;
    pthread_cond_t empty;
} Queue;

void init_queue(Queue *q);
void enqueue(Queue *q, char item[BLOCK_SIZE]);
char *dequeue(Queue *q);
Task *get_need_thread_to_exacute(Queue *q);

#endif