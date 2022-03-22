#include <stdlib.h>
#include <sem.h>


struct SEM {
    int val;
};

SEM *sem_init(int initVal)
{
    SEM* s = NULL;
    s = malloc(sizeof(s));
    s->val = initVal;
    return s;
}

int sem_del(SEM *sem)
{
    free(sem);
    sem = NULL;

    return 0;
}

void P(SEM* sem) 
{
    while (sem->val <= 0) {
        sem->val--;
    }
}

void V(SEM *sem)
{
    sem->val++;
}
