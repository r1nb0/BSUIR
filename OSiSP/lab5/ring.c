#include "ring.h"

#include <inttypes.h>

ring_node* constructor_node() {
    ring_node* buffer = (ring_node*)malloc(sizeof(ring_node));
    buffer->next = NULL;
    buffer->prev = NULL;
    buffer->flag_is_busy = false;
    return buffer;
}

ring_buffer* constructor_buffer() {
    ring_buffer* buffer = (ring_buffer*)malloc(sizeof(ring_buffer));
    buffer->begin = NULL;
    buffer->tail = NULL;
    return buffer;
}

void append(ring_buffer** __head) {
    if (__head == NULL)
        exit(-100);
    if (*__head == NULL) {
        *__head = constructor_buffer();
        (*__head)->begin = (*__head)->tail = constructor_node();
        (*__head)->begin->next = (*__head)->begin->prev = (*__head)->begin;
        return;
    }
    ring_node* buffer = constructor_node();
    if ((*__head)->begin->next == (*__head)->begin) {
        (*__head)->begin->next = (*__head)->begin->prev = buffer;
        buffer->next = buffer->prev = (*__head)->begin;
        return;
    }
    buffer->next = (*__head)->begin;
    buffer->prev = (*__head)->begin->prev;
    buffer->prev->next = buffer;
    (*__head)->begin->prev = buffer;
}

void erase(ring_buffer** __head) {

}

void add_message(ring_buffer* __head, const u_int8_t* __message) {
    if (__head == NULL) {
        printf("The ring is empty.\n");
        return;
    }
    if (__head->begin == NULL) {
        printf("There are 0 places in the ring.\n");
        return;
    }
    ring_node* __curr = __head->tail;
    if (__curr->flag_is_busy == true) {
        printf("No free places.\n");
        return;
    }
    for (size_t i = 0; i < LEN_MESSAGE; ++i) {
        __curr->message[i] = __message[i];
    }
    __curr->flag_is_busy = true;
    __head->tail = __head->tail->next;
    __head->produced++;
}

u_int8_t * extract_message(ring_buffer* __head) {
    if (__head == NULL) {
        printf("The ring is empty.\n");
        return NULL;
    }
    if (__head->begin == NULL) {
        printf("There are 0 places in the ring.\n");
        return NULL;
    }
    ring_node* __curr = __head->begin;
    if (__curr->flag_is_busy == false) {
        printf("No free places.\n");
        return NULL;
    }

    u_int8_t* __message = (u_int8_t*)calloc(LEN_MESSAGE, sizeof(u_int8_t));

    for (size_t i = 0; i < LEN_MESSAGE; ++i) {
        __message[i] = __curr->message[i];
    }
    __curr->flag_is_busy = false;
    __head->begin = __head->begin->next;
    __head->consumed++;
    return __message;
}

