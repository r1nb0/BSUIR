#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "ring.h"
#include "list.h"
#include <time.h>

#define BUFFER_SIZE 1

enum structure_of_message {
    TYPE = 0,
    HIGH_BYTE_HASH = 1,
    LOW_BYTE_HASH = 2,
    SIZE = 3,
    DATA_BEGIN = 4
};

sem_t* SEMAPHORE_EMPTY;
sem_t* SEMAPHORE_FILLED;
sem_t* SEMAPHORE_MUTEX;
bool FLAG_CONTINUE = true;


void clear_shared_memory(ring_shared_buffer* ring_queue) {
    int32_t curr;
    node_ring* buffer = shmat(ring_queue->shmid_begin, NULL, 0);
    while(buffer->shmid_next != ring_queue->shmid_tail) {
        curr = buffer->shmid_curr;
        int32_t shmid_next = buffer->shmid_next;
        shmdt(buffer);
        shmctl(curr, IPC_RMID, NULL);
        buffer = shmat(shmid_next, NULL, 0);
    }
    curr = ring_queue->shmid;
    shmdt(ring_queue);
    shmctl(curr, IPC_RMID, NULL);
}

void delete_all_child_proc(node_list* head) {
    //shmdt (ring_queue)
    while(head->next) {
        pid_t pid = pop_list(&head);
        kill(pid, SIGKILL);
    }
    printf("All child processes are deleted.\n");
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

void consumer(int32_t shmid) {
    ring_shared_buffer* queue = shmat(shmid, NULL, 0);
    do {
        sem_wait(SEMAPHORE_FILLED);
        sem_wait(SEMAPHORE_MUTEX);
        sleep(2);
        u_int8_t* message = extract_message(queue);
        sem_post(SEMAPHORE_MUTEX);
        display_message(message);
        free(message);
        printf("Consumed from CHILD with PID = %d\n", getpid());
        printf("Total messages retrieved = %lu\n", queue->consumed);
        sem_post(SEMAPHORE_EMPTY);
    }while(FLAG_CONTINUE);
    shmdt(queue);
}

void producer(int32_t shmid) {
    ring_shared_buffer* queue = shmat(shmid, NULL, 0);
    do {
        sem_wait(SEMAPHORE_EMPTY);
        sem_wait(SEMAPHORE_MUTEX);
        sleep(2);
        add_message(queue, generate_message());
        sem_post(SEMAPHORE_MUTEX);
        printf("Produced from CHILD with PID = %d\n", getpid());
        printf("Total ojbects created = %lu\n", queue->produced);
        sem_post(SEMAPHORE_FILLED);
    }while(FLAG_CONTINUE);
    shmdt(queue);
}


int main() {
    signal(SIGUSR1, handler_stop_proc);

    sem_unlink("SEMAPHORE_FILLED");
    sem_unlink("SEMAPHORE_EMPTY");
    sem_unlink("SEMAPHORE_MUTEX");

    SEMAPHORE_FILLED = sem_open("SEMAPHORE_FILLED", O_CREAT, 0777, 0);
    SEMAPHORE_EMPTY = sem_open("SEMAPHORE_EMPTY", O_CREAT, 0777, BUFFER_SIZE);
    SEMAPHORE_MUTEX = sem_open("SEMAPHORE_MUTEX", O_CREAT, 0777, 1);

    ring_shared_buffer* ring_queue = NULL;
    node_list* list_child_process = NULL;
    push_list(&list_child_process, getpid(), '-');

    for (size_t i = 0; i < BUFFER_SIZE; ++i)
        append(&ring_queue);

    printf("Shmid segment : %d\n", ring_queue->shmid);

    int status;

    do {
        char ch = getchar();
        switch(ch) {
            case 'p' : {
                pid_t pid = fork();
                if (pid == 0) { producer(ring_queue->shmid); }
                else { push_list(&list_child_process, pid, 'P'); }
                break;
            }
            case 'c' : {
                pid_t pid = fork();
                if (pid == 0) {  consumer(ring_queue->shmid); }
                else { push_list(&list_child_process, pid, 'C');   }
                break;
            }
            case 'l' : {
                display_list(list_child_process);
                break;
            }
            case 'k' : {
                size_t num;
                scanf("%lu", &num);
                if (num == 0) { printf("This process is not a child process.\n"); }
                 else {
                     pid_t pid = erase_list(&list_child_process, num);
                     if (pid != -1) kill(pid, SIGUSR1);
                 }
                break;
            }
            case 'q' : {
                delete_all_child_proc(list_child_process);
                clear_shared_memory(ring_queue);
                FLAG_CONTINUE = false;
                break;
            }
            default : {
                printf("Incorrect input.\n");
                fflush(stdin); break;
            }
        }
        waitpid(-1, &status, WNOHANG);
        getchar();
    }while(FLAG_CONTINUE);

    sem_unlink("SEMAPHORE_FILLED");
    sem_unlink("SEMAPHORE_EMPTY");
    sem_unlink("SEMAPHORE_MUTEX");

    sem_close(SEMAPHORE_MUTEX);
    sem_close(SEMAPHORE_EMPTY);
    sem_close(SEMAPHORE_FILLED);

    return 0;
}
