#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "sem.h"
#include "bbuffer.h"

void *test1();
void *test2();

int main() {

   pthread_t thread1, thread2;

   SEM *sem = sem_init(0);

   pthread_create( &thread1, NULL, test1, sem);
   pthread_create( &thread2, NULL, test2, sem);
   pthread_join( thread1, NULL);
   pthread_join( thread2, NULL);

   return 0;
}

void *test1(SEM *sem)
{
    printf("Thread 1 test, with input %d\n", sem->val);
    P(sem);
    
}

void *test2(SEM *sem)
{
    printf("Thread 2 test\n");
    V(sem);
}