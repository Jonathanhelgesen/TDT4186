#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "sem.h"

typedef struct SEM
{
    int count, waiting; // Trenger man waiting/wakeups? Ser det er implementert noen steder.
    pthread_mutex_t *mutex;
    pthread_cond_t *cv; // Conditional value
}SEM;

/* Creates a new semaphore.
 *
 * This function creates a new semaphore. If an error occurs during the
 * initialization, the implementation shall free all resources already
 * allocated so far.
 *
 * Parameters:
 *
 * initVal      the initial value of the semaphore
 *
 * Returns:
 *
 * handle for the created semaphore, or NULL if an error occured.
 */
SEM *sem_init(int initVal)
{
    if (initVal < 0)
    {
        return NULL;
    }
    SEM *semaphore = malloc(sizeof(SEM));
    pthread_mutex_t *mutex = malloc(sizeof(pthread_mutex_t));
    pthread_cond_t *cv = malloc(sizeof(pthread_cond_t));

    if (semaphore == NULL || mutex == NULL || cv == NULL)
    {
        printf("Not able to allocate memory\n");
        if (semaphore != NULL) free(semaphore);
        if (mutex != NULL) free(mutex);
        if (cv != NULL) free(cv);
        return NULL;
    }

    // Initilize mutex, check for errors
    if (pthread_mutex_init(mutex, NULL) != 0)
    {
        printf("Error when making mutex\n");
        free(semaphore);
        free(mutex);
        free(cv);
        return NULL;
    }

    // Initilize conditional value, check for errora
    if (pthread_cond_init(cv, NULL) != 0)
    {
        printf("Error when making conditional value");
        free(semaphore);
        free(mutex);
        free(cv);
        return NULL;
    }

    semaphore->count = initVal;
    // semaphore->waiting = 0;
    semaphore->mutex = mutex;
    semaphore->cv = cv;
    return semaphore;
}

/* Destroys a semaphore and frees all associated resources.
 *
 * Parameters:
 *
 * sem           handle of the semaphore to destroy
 *
 * Returns:
 *
 * 0 on success, negative value on error.
 *
 * In case of an error, not all resources may have been freed, but
 * nevertheless the semaphore handle must not be used any more.
 */
int sem_del(SEM *sem)
{
    if (sem == NULL)
    {
        return -1;
    }
    // TODO: Propper error handling
    free(sem->mutex);
    free(sem->cv);
    free(&sem->count);
    free(sem);
    return 0;
}

    /* P (wait) operation.
     *
     * Attempts to decrement the semaphore value by 1. If the semaphore value
     * is 0, the operation blocks until a V operation increments the value and
     * the P operation succeeds.
     *
     * Parameters:
     *
     * sem           handle of the semaphore to decrement
     */
    void P(SEM * sem)
    {
        pthread_mutex_lock(sem->mutex);
        sem->count--;

        while (sem->count < 0)
        {
            pthread_cond_wait(sem->cv, sem->mutex);
        }
        pthread_cond_signal(sem->cv);
        pthread_mutex_unlock(sem->mutex);
    }

    /* V (signal) operation.
     *
     * Increments the semaphore value by 1 and notifies P operations that are
     * blocked on the semaphore of the change.
     *
     * Parameters:
     *
     * sem           handle of the semaphore to increment
     */
    void V(SEM * sem)
    {
        pthread_mutex_lock(sem->mutex);
        sem->count++;
        pthread_cond_signal(sem->cv);
        pthread_mutex_unlock(sem->mutex);
    }