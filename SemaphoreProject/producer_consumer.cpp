/* producer_consumer.cpp
   Solves the bounded buffer problem using pthreads and semaphores.
   Run: ./producer_consumer <sleep_time> <num_producers> <num_consumers> */

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "buffer.h"

using namespace std;

/* the shared buffer treated as a circular queue */
buffer_item buffer[BUFFER_SIZE];
int in  = 0; /* where the producer puts the next item */
int out = 0; /* where the consumer takes the next item */

/* semaphores track how many empty/full slots are in the buffer */
sem_t emptySpaces; /* starts at BUFFER_SIZE, decreases as items are added */
sem_t fullSpaces;  /* starts at 0, increases as items are added */
pthread_mutex_t mutex; /* makes sure only one thread touches the buffer at a time */

int insert_item(buffer_item item) {
    buffer[in] = item;
    in = (in + 1) % BUFFER_SIZE; /* wrap around to the front when we hit the end */
    return 0;
}

int remove_item(buffer_item *item) {
    *item = buffer[out];
    out = (out + 1) % BUFFER_SIZE; /* wrap around to the front when we hit the end */
    return 0;
}

void *producer(void *param) {
    (void)param;
    buffer_item item;

    while (1) {
        sleep(rand() % 3 + 1); /* wait a random amount of time before producing */

        item = rand(); /* generate a random number to put in the buffer */

        sem_wait(&emptySpaces);       /* wait if there are no empty slots */
        pthread_mutex_lock(&mutex);   /* lock so no other thread interferes */

        if (insert_item(item))
            cerr << "Producer error inserting item" << endl;
        else
            cout << "Producer produced item: " << item << endl;

        pthread_mutex_unlock(&mutex); /* done, let other threads in */
        sem_post(&fullSpaces);        /* tell consumers there is a new item ready */
    }

    return NULL;
}

void *consumer(void *param) {
    (void)param;
    buffer_item item;

    while (1) {
        sleep(rand() % 3 + 1); /* wait a random amount of time before consuming */

        sem_wait(&fullSpaces);        /* wait if there are no items to consume */
        pthread_mutex_lock(&mutex);   /* lock so no other thread interferes */

        if (remove_item(&item))
            cerr << "Consumer error removing item" << endl;
        else
            cout << "Consumer consumed item: " << item << endl;

        pthread_mutex_unlock(&mutex); /* done, let other threads in */
        sem_post(&emptySpaces);       /* tell producers there is a new empty slot */
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <sleep_time> <num_producers> <num_consumers>" << endl;
        return 1;
    }

    int sleep_time    = atoi(argv[1]);
    int num_producers = atoi(argv[2]);
    int num_consumers = atoi(argv[3]);

    /* set up semaphores and mutex before creating any threads */
    sem_init(&emptySpaces, 0, BUFFER_SIZE); /* all slots start empty */
    sem_init(&fullSpaces,  0, 0);           /* no items in buffer yet */
    pthread_mutex_init(&mutex, NULL);

    srand((unsigned int)getpid()); /* seed random number generator */

    /* create arrays to hold the thread IDs */
    pthread_t prod_threads[num_producers];
    pthread_t cons_threads[num_consumers];

    /* start producer threads */
    for (int i = 0; i < num_producers; i++)
        pthread_create(&prod_threads[i], NULL, producer, NULL);

    /* start consumer threads */
    for (int i = 0; i < num_consumers; i++)
        pthread_create(&cons_threads[i], NULL, consumer, NULL);

    /* let everything run for the given number of seconds, then quit */
    sleep(sleep_time);
    cout << "Time's up. Exiting." << endl;

    /* clean up */
    sem_destroy(&emptySpaces);
    sem_destroy(&fullSpaces);
    pthread_mutex_destroy(&mutex);

    return 0;
}
