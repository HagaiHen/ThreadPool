#include "queue.h"

char dequeue_data[1024];

void init_queue(Queue *q)
{

    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    q->need_thread_to_exacute = NULL;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->empty, NULL);
}

void enqueue(Queue *q, char item[BLOCK_SIZE])
{
    Task *new_task = (Task *)malloc(sizeof(Task));

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
        q->tail = q->tail->next;
    }
    q->size++;
    if (q->size == 1 && q->need_thread_to_exacute == NULL)
    {
        q->need_thread_to_exacute = q->head;
    }

    // pthread_cond_signal(&q->empty);
    pthread_mutex_unlock(&q->lock);
}

char *dequeue(Queue *q)
{
    pthread_mutex_lock(&q->lock);
    while (q->size == 0)
    {
        pthread_cond_wait(&q->empty, &q->lock);
    }
    strcpy(dequeue_data, q->head->data);

    if (q->head->next != NULL)
    {
        q->head = q->head->next;
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
