#include <stdlib.h>
#include "bbuffer.h"
#include "sem.h"

BNDBUF *bb_init(unsigned int size)
{

    BNDBUF *buf = NULL;
    buf = malloc(sizeof(BNDBUF));

    buf->mem_start = (int *) malloc(size*sizeof(int));
    buf->size = size;

    buf->head = sem_init(buf->mem_start);
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
    int value = *(bb->head);

    pthread_mutex_lock(&(bb->mutex));
    bb->head = (bb->head++) % bb->size;
    pthread_mutex_unlock(&(bb->mutex));

    return value;
}

void bb_add(BNDBUF *bb, int fd)
{
    if (bb->count > bb->size) {

    }
    V(bb->count);

}

