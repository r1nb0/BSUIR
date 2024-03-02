#ifndef RING_H
#define RING_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/shm.h>
#include <string.h>

#define LEN_MESSAGE 256

typedef struct node_ring {
    int32_t shmid_curr;
    int32_t shmid_next;
    int32_t shmid_prev;
    char message[LEN_MESSAGE];
    bool flag_is_busy;
}node_ring;

typedef struct ring_shared_buffer {
    key_t key_ring;
    int32_t shmid;
    size_t consumed;
    size_t produced;
    int32_t shmid_begin;
    int32_t shmid_tail;
}ring_shared_buffer;

node_ring* constructor_node();
ring_shared_buffer* constructor_ring();
void append(ring_shared_buffer**);
void add_message(ring_shared_buffer*, const char*);
char* extract_message(ring_shared_buffer*);

#endif //RING_H