#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "sem.h"
#include "bbuffer.h"

typedef struct BNDBUF
{
    pthread_mutex_t *lock;
    SEM *empty, *full;
    int *fds;               // File descriptors
    int position;           // Keeps track the start, end and total number of entries in buffer
} BNDBUF;

/* Creates a new Bounded Buffer.
 *
 * This function creates a new bounded buffer and all the helper data
 * structures required by the buffer, including semaphores for
 * synchronization. If an error occurs during the initialization the
 * implementation shall free all resources already allocated by then.
 *
 * Parameters:
 *
 * size     The number of integers that can be stored in the bounded buffer.
 *
 * Returns:
 *
 * handle for the created bounded buffer, or NULL if an error occured.
 */
BNDBUF *bb_init(unsigned int size)
{
    BNDBUF *bndbuf = malloc(sizeof(BNDBUF));
    int *fds_pointer = malloc(size * sizeof(int));
    SEM *empty = malloc(sizeof(SEM));
    SEM *full = malloc(sizeof(SEM));
    pthread_mutex_t *lock = malloc(sizeof(pthread_mutex_t));

    if (bndbuf == NULL || fds_pointer == NULL || empty == NULL || full == NULL || lock == NULL)
    {
        printf("Not able to allocate memory\n");
        if (bndbuf != NULL)
            free(bndbuf);
        if (fds_pointer != NULL)
            free(fds_pointer);
        if (empty != NULL)
            free(empty);
        if (full != NULL)
            free(full);
        if (lock != NULL)
            free(lock);
        return NULL;
    }

    if (pthread_mutex_init(lock, NULL) != 0)
    {
        printf("Error when making mutex\n");
        free(bndbuf);
        free(fds_pointer);
        free(empty);
        free(full);
        free(lock);
        return NULL;
    }

    bndbuf->fds = fds_pointer;
    bndbuf->empty = sem_init(size);
    bndbuf->full = sem_init(0);
    bndbuf->lock = lock;
    bndbuf->position = 0;
    return bndbuf;
}

/* Destroys a Bounded Buffer.
 *
 * All resources associated with the bounded buffer are released.
 *
 * Parameters:
 *
 * bb       Handle of the bounded buffer that shall be freed.
 */
void bb_del(BNDBUF *bb)
{
    free(bb->empty);
    free(bb->full);
    free(bb->fds);
    free(bb->lock);
    free(bb);
}

/* Add an element to the bounded buffer.
 *
 * This function adds an element to the bounded buffer. If the bounded
 * buffer is full, the function blocks until an element is removed from
 * the buffer.
 *
 * Parameters:
 *
 * bb     Handle of the bounded buffer.
 * fd     Value that shall be added to the buffer.
 *
 * Returns:
 *
 * the int element
 */
void bb_add(BNDBUF *bb, int fd)
{
    // Make empty-semaphore wait
    P(bb->empty);
    pthread_mutex_lock(bb->lock);
    // Entering critical section
    bb->fds[bb->position] = fd;
    bb->position++;
    pthread_mutex_unlock(bb->lock);
    printf("Added %d\n", fd);
    // Send signal to full-semaphore
    V(bb->full);
    printf("Lengde på buffer: %d\n", bb->position);
}

/* Retrieve an element from the bounded buffer.
 *
 * This function removes an element from the bounded buffer. If the bounded
 * buffer is empty, the function blocks until an element is added to the
 * buffer.
 *
 * Parameters:
 *
 * bb         Handle of the bounded buffer.
 *
 * Returns:
 *
 * the int element
 */
int bb_get(BNDBUF *bb)
{
    // Make full-semaphore wait
    P(bb->full);
    pthread_mutex_lock(bb->lock);
    // Entering critical section
    int fd = bb->fds[bb->position - 1];
    bb->position--;
    pthread_mutex_unlock(bb->lock);
    // Send signal to empty-semaphore
    V(bb->empty);
    printf("Got %d\n", fd);
    printf("Lengde på buffer: %d\n", bb->position);
    return fd;
}

struct readThreadParams
{
    int fd;
    BNDBUF *bb;
};

void *parse_bb_add(void *parameters)
{
    struct readThreadParams *params = parameters;
    bb_add(params->bb, params->fd);
}

void *parse_bb_get(void *pbb)
{
    BNDBUF *bb =(BNDBUF*) pbb;
    bb_get(bb);
}

int main()
{
    BNDBUF *bb = bb_init(10);
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    int THREAD_NUM = 8;
    pthread_t th[THREAD_NUM];
    int numbers[THREAD_NUM];
    int i;
    for (i = 0; i < 100; i++) {
        numbers[i] = i * 10;
    }

    for (i = 0; i < THREAD_NUM; i++)
    {
        if (i % 2 == 0 || i % 3 == 0)
        {
            struct readThreadParams readParams;
            readParams.bb = bb;
            readParams.fd = numbers[i];
            pthread_mutex_lock(&mutex);
            if (pthread_create(&th[i], NULL, &parse_bb_add, &readParams) != 0)
            {
                perror("Failed to create thread");
            }
            pthread_mutex_unlock(&mutex);
            
        }
        else
        {
            if (pthread_create(&th[i], NULL, &parse_bb_get, bb) != 0)
            {
                perror("Failed to create thread");
            }
        }
    }
    for (i = 0; i < 100; i++)
    {
        if (pthread_join(th[i], NULL) != 0)
        {
            perror("Failed to join thread");
        }
    }
    bb_del(bb);
    return 0;
}