#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define BLOCK_SIZE 1024
#define ENCRYPT 1
#define DECRYPT 2

char dequeue_data[1024];

typedef struct task
{
    char data[BLOCK_SIZE];
    int key;
    int job;
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

void init_queue(Queue *q)
{

    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    q->need_thread_to_exacute = NULL;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->empty, NULL);
}

void enqueue(Queue *q, char item[BLOCK_SIZE], int key, int job)
{
    Task *new_task = (Task *)malloc(sizeof(Task));
    new_task->job = job;
    new_task->key = key;
    strcpy(new_task->data, item);
    new_task->next = NULL;

    pthread_mutex_lock(&q->lock);
    if (q->tail == NULL)
    {
        q->head = new_task;
        q->tail = new_task;
    }
    else
    {
        q->tail->next = new_task;
        q->tail = new_task;
    }
    q->size++;
    if (q->need_thread_to_exacute == NULL)
    {
        q->need_thread_to_exacute = new_task;
    }

    pthread_cond_signal(&q->empty);
    pthread_mutex_unlock(&q->lock);
}

char *dequeue(Queue *q)
{
    pthread_mutex_lock(&q->lock);
    while (q->size == 0)
    {
        pthread_cond_wait(&q->empty, &q->lock);
    }
    bzero(dequeue_data, BLOCK_SIZE);
    strcpy(dequeue_data, q->head->data);

    Task *temp = q->head;
    if (q->head->next != NULL)
    {
        q->head = q->head->next;
        free(temp);
    }
    q->size--;

    pthread_mutex_unlock(&q->lock);
    return dequeue_data;
}

Task *get_need_thread_to_exacute(Queue *q)
{
    pthread_mutex_lock(&q->lock);

    Task *ans = q->need_thread_to_exacute;

    if (q->need_thread_to_exacute->next != NULL)
    {
        q->need_thread_to_exacute = q->need_thread_to_exacute->next;
    }
    pthread_mutex_unlock(&q->lock);
    return ans;
}
