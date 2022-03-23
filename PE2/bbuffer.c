#include <stdlib.h>
#include "bbuffer.h"
#include "sem.h"

struct BNDBUF {
    int *mem_start;
    int *mem_end;
    int *head;
    int *tail;
};

BNDBUF *bb_init(unsigned int size)
{

    BNDBUF *buf = NULL;
    buf->mem_start = (int *) malloc(size*sizeof(int));
    buf->mem_end = buf->mem_start + size;

    buf->head = buf->mem_start;
    buf->tail = buf->mem_start;

    return buf;
}

void bb_del(BNDBUF *bb)
{
    free(bb);
    bb = NULL;
}

int bb_get(BNDBUF *bb) 
{
    int value = *(bb->head);
    if (bb->head >= bb->mem_end) {
        bb->head = bb->mem_start;
    } else {
        bb->head++;
    }

    return value;
}

void bb_add(BNDBUF *bb, int fd)
{
    if (bb->tail >= bb->mem_end) {
        bb->tail = bb->mem_start;
    } else {
        bb->tail++;
    }
    
    *(bb->tail) = fd;
}

