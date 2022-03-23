#include "bbuffer.h"
#include "sem.h"
#include "pthread.h"

void *test1(SEM* sem);
void *test2(SEM* sem);

int main() {

   pthread_t thread1, thread2;

   SEM *sem = sem_init(5);

   pthread_create( &thread1, NULL, test1, sem);
   pthread_create( &thread2, NULL, test2, sem);
   pthread_join( thread1, NULL);
   pthread_join( thread2, NULL);

   return 0;
}

void *test1(SEM *sem)
{
    P(sem);
}

void *test2(SEM *sem)
{
    V(sem);
}