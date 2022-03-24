#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "bbuffer.h"
#include "sem.h"

BNDBUF *bb_init(unsigned int size)
{

    BNDBUF *buf = NULL;
    buf = malloc(sizeof(BNDBUF));

    buf->mem_start = (int *) malloc(size*sizeof(int));
    buf->size = size;


    buf->head = buf->mem_start;
    buf->tail = buf->mem_start;
    buf->count = sem_init(0);

    return buf;
}

void bb_del(BNDBUF *bb)
{
    free(bb);
    bb = NULL;
}

int bb_get(BNDBUF *bb) 
{
    P(bb->count);
    pthread_mutex_lock(&(bb->lock));

    int value = *(bb->head);
    if (++bb->head > (bb->mem_start + bb->size))
        bb->head = bb->mem_start;

    // printf("Head pointing at %p\n", bb->head);

    pthread_cond_signal(&(bb->cond));
    pthread_mutex_unlock(&(bb->lock));

    return value;
}

void bb_add(BNDBUF *bb, int fd)
{

    pthread_mutex_lock(&(bb->lock));
    while (bb->count->val >= bb->size)
        pthread_cond_wait(&(bb->cond), &(bb->lock));

    *(bb->tail) = fd;
    if (++bb->tail > (bb->mem_start + bb->size))
        bb->tail = bb->mem_start;

    // printf("Tail pointing at %p\n", bb->tail);

    pthread_mutex_unlock(&(bb->lock));
    V(bb->count);
}

