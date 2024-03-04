#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include "ring.h"
#include <pthread.h>
#include "list.h"

#define BUFFER_SIZE 6

enum structure_of_message {
    TYPE = 0,
    HIGH_BYTE_HASH = 1,
    LOW_BYTE_HASH = 2,
    SIZE = 3,
    DATA_BEGIN = 4
};

sem_t* SEMAPHORE_EMPTY;
sem_t* SEMAPHORE_FILLED;
pthread_mutex_t mutex;
ring_buffer* queue = NULL;
_Thread_local bool FLAG_CONTINUE = true;


u_int16_t control_sum(const u_int8_t*, size_t);
void producer();
void consumer();
u_int8_t* generate_message();
void handler_stop_proc();
void display_message(const u_int8_t* message);

int main(void) {

    node_list* list_of_ptreads = NULL;

    sem_unlink("SEMAPHORE_FILLED");
    sem_unlink("SEMAPHORE_EMPTY");
    sem_unlink("SEMAPHORE_MUTEX");


    SEMAPHORE_FILLED = sem_open("SEMAPHORE_FILLED", O_CREAT, 0777, 0);
    SEMAPHORE_EMPTY = sem_open("SEMAPHORE_EMPTY", O_CREAT, 0777, BUFFER_SIZE);
    pthread_mutex_init(&mutex, NULL);

    for (size_t i = 0; i < BUFFER_SIZE; ++i)
        append(&queue);

    bool flag = true;
    do {
        char ch = getchar();
        switch(ch) {
            case 'p' : {
                pthread_t pthread_producer;
                pthread_create(&pthread_producer, NULL, producer, NULL);
                printf("%lu\n", pthread_producer);
                push_list(&list_of_ptreads, pthread_producer, 'P');
                break;
            }
            case 'c' : {
                pthread_t pthread_consumer;
                pthread_create(&pthread_consumer, NULL, consumer, NULL);
                push_list(&list_of_ptreads, pthread_consumer, 'C');
                break;
            }
            case 'l' : {
                display_list(list_of_ptreads);
                break;
            }
            case 'k' : {
                size_t num;
                scanf("%lu", &num);
                pthread_t pthread_id;
                printf("%lu\n", pthread_id);
                if (erase_list(&list_of_ptreads, num, &pthread_id)) pthread_kill(pthread_id, SIGUSR1);
                break;
            }
            default : {flag = false; break; }
        }
        getchar();
    }while(flag);

    sem_unlink("SEMAPHORE_FILLED");
    sem_unlink("SEMAPHORE_EMPTY");

    sem_close(SEMAPHORE_EMPTY);
    sem_close(SEMAPHORE_FILLED);

    pthread_mutex_destroy(&mutex);
    return 0;
}

u_int16_t control_sum(const u_int8_t* data, size_t length) {
    u_int16_t hash = 0;
    for (size_t i = 0; i < length; ++i) {
        hash += data[i];
    }
    return hash;
}

u_int8_t* generate_message() {
    srand(time(NULL));
    u_int8_t* result_message = (u_int8_t*)calloc(LEN_MESSAGE , sizeof(u_int8_t));
    size_t size = 0;
    while(size == 0) size = rand() % 257;
    for (size_t i = DATA_BEGIN; i < size; ++i) {
        result_message[i] = rand() % 256;
    }
    u_int16_t hash = control_sum(result_message, size);
    result_message[TYPE] = 1;
    result_message[HIGH_BYTE_HASH] = (hash >> 8) & 0xFF;
    result_message[LOW_BYTE_HASH] = hash & 0xFF;
    result_message[SIZE] = size;
    return result_message;
}

void display_message(const u_int8_t* message) {
    for (size_t i = 0; i < message[SIZE]; ++i)
        printf("%02X", message[i]);
    printf("\n");
}

void handler_stop_proc() {
    FLAG_CONTINUE = false;
}

void consumer() {
    signal(SIGUSR1, handler_stop_proc);
    do {
        sem_wait(SEMAPHORE_FILLED);
        pthread_mutex_lock(&mutex);
        sleep(2);
        u_int8_t* message = extract_message(queue);
        pthread_mutex_unlock(&mutex);
        sem_post(SEMAPHORE_EMPTY);
        display_message(message);
        free(message);
        printf("Consumed from pthread with id = %lu\n", pthread_self());
        printf("Total messages retrieved = %lu\n", queue->consumed);
    }while(FLAG_CONTINUE);
}

void producer() {
    signal(SIGUSR1, handler_stop_proc);
    do {
        sem_wait(SEMAPHORE_EMPTY);
        pthread_mutex_lock(&mutex);
        sleep(2);
        u_int8_t* new_message = generate_message();
        add_message(queue, new_message);
        pthread_mutex_unlock(&mutex);
        sem_post(SEMAPHORE_FILLED);
        free(new_message);
        printf("Produced from pthread with id = %lu\n", pthread_self());
        printf("Total ojbects created = %lu\n", queue->produced);
    }while(FLAG_CONTINUE);
}