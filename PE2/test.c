#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "sem.h"
#include "bbuffer.h"

void *test1();
void *test2();

int main() {


   pthread_t thread1, thread2;

   BNDBUF *buf = bb_init(5);

   pthread_create( &thread1, NULL, test1, buf);
   pthread_create( &thread2, NULL, test2, buf);
   pthread_join( thread1, NULL);
   pthread_join( thread2, NULL);

   bb_del(buf);

   return 0;
}

void *test1(BNDBUF *buf)
{
    for (int i = 0; i < 3; i++)
    {
        printf("Thread 1 adding %d to buffer\n", i);
        bb_add(buf, i);
    }
    
    
}

void *test2(BNDBUF *buf)
{
    for (int i = 0; i < 8; i++)
    {
        printf("Thread 2 getting from buffer\n");
        int val = bb_get(buf);
        printf("Got number %d\n", val);
    }
    

}