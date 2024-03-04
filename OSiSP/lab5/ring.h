#ifndef RING_H
#define RING_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdbool.h>

#define LEN_MESSAGE 256

typedef struct ring_node {
    struct ring_node* next;
    struct ring_node* prev;
    u_int8_t message[LEN_MESSAGE];
    bool flag_is_busy;
}ring_node;

typedef struct ring_buffer {
    ring_node* begin;
    ring_node* tail;
    size_t produced;
    size_t consumed;
}ring_buffer;

ring_node* constructor_node();
ring_buffer* constructor_buffer();
void append(ring_buffer**);
void erase(ring_buffer**);
void add_message(ring_buffer*, const u_int8_t*);
u_int8_t* extract_message(ring_buffer*);

#endif //RING_H
