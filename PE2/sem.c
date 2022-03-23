#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "sem.h"

SEM *sem_init(int initVal)
{
    SEM *s = NULL;
    s = malloc(sizeof(SEM));
    s->val = initVal;

    pthread_mutex_init(&(s->lock), NULL);
    pthread_cond_init(&(s->cond), NULL);

    return s;
}

int sem_del(SEM *sem)
{
    free(sem);
    sem = NULL;

    return 0;
}

void P(SEM *sem)
{
    pthread_mutex_lock(&(sem->lock));
    while (sem->val <= 0)
        pthread_cond_wait(&(sem->cond), &(sem->lock));
    sem->val--;
    printf("Decrementing semaphore value, value: %d\n", sem->val);
    pthread_mutex_unlock(&(sem->lock));
}

void V(SEM *sem)
{
    pthread_mutex_lock(&(sem->lock));
    sem->val++;
    printf("Incrementing semaphore value, value: %d\n", sem->val);
    pthread_cond_signal(&(sem->cond));
    pthread_mutex_unlock(&(sem->lock));
}
