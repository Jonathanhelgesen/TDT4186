#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "sem.h"

SEM *sem_init(int initVal)
{
    SEM *s = NULL;
    s = malloc(sizeof(SEM));
    s->val = initVal;

    pthread_mutex_init(&(s->cond_mutex), NULL);
    pthread_mutex_init(&(s->count_mutex), NULL);
    pthread_cond_init(&(s->cond_condition), NULL);

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
    pthread_mutex_lock(&(sem->cond_mutex));
    while (sem->val <= 0)
    {
        pthread_cond_wait(&(sem->cond_condition), &(sem->cond_mutex));
    }
    pthread_mutex_unlock(&(sem->cond_mutex));

    pthread_mutex_lock(&(sem->count_mutex));
    sem->val--;
    printf("Decrementing semaphore value, value: %d\n", sem->val);
    pthread_mutex_unlock(&(sem->count_mutex));
}

void V(SEM *sem)
{
    pthread_mutex_lock(&(sem->cond_mutex));
    pthread_cond_signal(&(sem->cond_condition));
    pthread_mutex_unlock(&(sem->cond_mutex));

    pthread_mutex_lock(&(sem->count_mutex));
    sem->val++;
    printf("Incrementing semaphore value, value: %d\n", sem->val);
    pthread_mutex_unlock(&(sem->count_mutex));
}
