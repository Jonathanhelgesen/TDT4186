#include <stdlib.h>
#include "bbuffer.h"
#include "sem.h"

struct BNDBUF {
    int *mem_start;
    int *mem_end;
    int *data_start;
    int *data_end;
};

BNDBUF *bb_init(unsigned int size)
{

    BNDBUF *buf = NULL;
    buf->mem_start = (int *) malloc(size*sizeof(int));
    buf->mem_end = buf->mem_start + size;

    buf->data_start = buf->mem_start;
    buf->data_end = buf->mem_start;

}

void bb_del(BNDBUF *bb)
{
    free(bb);
    bb = NULL;
}

int bb_get(BNDBUF *bb) 
{
    int value = *(bb->data_start);
    if (bb->data_start >= bb->mem_end) {
        bb->data_start = bb->mem_start;
    } else {
        bb->data_start++;
    }

    return value;
}

void bb_add(BNDBUF *bb, int fd)
{
    if (bb->data_end >= bb->mem_end) {
        bb->data_end = bb->mem_start;
    } else {
        bb->data_end++;
    }
    
    *(bb->data_end) = fd;
}

