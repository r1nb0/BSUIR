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






// #ifndef RING_H
// #define RING_H
//
// #include <stdio.h>
// #include <stdlib.h>
// #include <stdbool.h>
// #include <sys/shm.h>
// #include <string.h>
//
// #define LEN_MESSAGE 256
//
// typedef struct node {
//     int shmid_curr;
//     int shmid_next;
//     int shmid_prev;
//     char message[LEN_MESSAGE];
//     bool flag_is_busy;
// }node;
//
// typedef struct ring_buffer {
//     key_t key_ring;
//     int shmid;
//     size_t consumed;
//     size_t produced;
//     node* begin;
//     node* tail;
// }ring_buffer;
//
// node* constructor_node();
// ring_buffer* constructor_ring();
// void display(ring_buffer*);
// void append(ring_buffer**);
// void add_message(ring_buffer*, const char*);
// char* extract_message(ring_buffer*);
// void get_count_produced();
// void get_count_consumed();
//
// #endif //RING_H

